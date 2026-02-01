#include "core/system/calibration_serial.h"

#include <Arduino.h>
#include <string.h>

#include "hardware/color_sensor.h"
#include "hardware/ir_sensor.h"
#include "hardware/ultrasonic.h"

namespace {
const long kDefaultBaud = 115200;

const int kIrSamples = 20;
const int kColorSamples = 20;
const int kUltraSamples = 15;

const int kUltraPoints[] = {10, 20, 30, 40, 60, 80};
const int kUltraPointCount = sizeof(kUltraPoints) / sizeof(kUltraPoints[0]);

enum CalState {
  CAL_IDLE,
  CAL_IR_PROFILE_WAIT_CARDBOARD,
  CAL_IR_PROFILE_WAIT_RED,
  CAL_IR_PROFILE_WAIT_BLUE,
  CAL_IR_PROFILE_WAIT_GREEN,
  CAL_COLOR_WAIT_WHITE,
  CAL_COLOR_WAIT_BLACK,
  CAL_COLOR_WAIT_CARDBOARD,
  CAL_COLOR_WAIT_RED,
  CAL_COLOR_WAIT_BLUE,
  CAL_COLOR_WAIT_GREEN,
  CAL_ULTRA_WAIT_POINT
};

CalState g_state = CAL_IDLE;
int g_ultra_index = 0;
int g_ultra_raw[kUltraPointCount];
hardware::ColorTargets g_color_targets_temp;

struct IrProfile {
  int cardboard_left;
  int cardboard_right;
  int red_left;
  int red_right;
  int blue_left;
  int blue_right;
  int green_left;
  int green_right;
};

IrProfile g_ir_profile_temp;

char g_cmd_buf[32];
int g_cmd_len = 0;

// Averages multiple raw reads for a stable sample.
int avgRaw(int (*readFn)(), int samples) {
  long sum = 0;
  for (int i = 0; i < samples; ++i) {
    sum += readFn();
  }
  return static_cast<int>(sum / samples);
}

// Averages multiple raw RGB reads into one sample.
hardware::ColorRaw avgColorRaw(int samples) {
  long sum_r = 0, sum_g = 0, sum_b = 0;
  for (int i = 0; i < samples; ++i) {
    hardware::ColorRaw c = hardware::colorReadRawRgb();
    sum_r += c.r;
    sum_g += c.g;
    sum_b += c.b;
  }
  hardware::ColorRaw out;
  out.r = static_cast<int>(sum_r / samples);
  out.g = static_cast<int>(sum_g / samples);
  out.b = static_cast<int>(sum_b / samples);
  return out;
}

// Averages multiple normalized reads into one target sample.
hardware::ColorTarget avgColorNorm(int samples) {
  float sum_r = 0, sum_g = 0, sum_b = 0;
  for (int i = 0; i < samples; ++i) {
    hardware::ColorNorm n = hardware::colorReadNormalized();
    sum_r += n.r;
    sum_g += n.g;
    sum_b += n.b;
  }
  hardware::ColorTarget out;
  out.r = sum_r / samples;
  out.g = sum_g / samples;
  out.b = sum_b / samples;
  return out;
}

// Prints IR calibration as a single JSON line.
void printIrJson(const hardware::IrCal &cal) {
  Serial.print("{\"sensor\":\"ir\"");
  Serial.print(",\"left_white\":");
  Serial.print(cal.left_white);
  Serial.print(",\"left_black\":");
  Serial.print(cal.left_black);
  Serial.print(",\"right_white\":");
  Serial.print(cal.right_white);
  Serial.print(",\"right_black\":");
  Serial.print(cal.right_black);
  Serial.print(",\"left_thresh\":");
  Serial.print(cal.left_thresh);
  Serial.print(",\"right_thresh\":");
  Serial.print(cal.right_thresh);
  Serial.println("}");
}

// Prints color calibration as a single JSON line.
void printColorJson(const hardware::ColorCal &cal,
                    const hardware::ColorTargets &targets) {
  Serial.print("{\"sensor\":\"color\"");
  Serial.print(",\"white_r\":");
  Serial.print(cal.white.r);
  Serial.print(",\"white_g\":");
  Serial.print(cal.white.g);
  Serial.print(",\"white_b\":");
  Serial.print(cal.white.b);
  Serial.print(",\"black_r\":");
  Serial.print(cal.black.r);
  Serial.print(",\"black_g\":");
  Serial.print(cal.black.g);
  Serial.print(",\"black_b\":");
  Serial.print(cal.black.b);
  Serial.print(",\"cardboard_r\":");
  Serial.print(targets.cardboard.r, 6);
  Serial.print(",\"cardboard_g\":");
  Serial.print(targets.cardboard.g, 6);
  Serial.print(",\"cardboard_b\":");
  Serial.print(targets.cardboard.b, 6);
  Serial.print(",\"target_black_r\":");
  Serial.print(targets.black.r, 6);
  Serial.print(",\"target_black_g\":");
  Serial.print(targets.black.g, 6);
  Serial.print(",\"target_black_b\":");
  Serial.print(targets.black.b, 6);
  Serial.print(",\"red_r\":");
  Serial.print(targets.red.r, 6);
  Serial.print(",\"red_g\":");
  Serial.print(targets.red.g, 6);
  Serial.print(",\"red_b\":");
  Serial.print(targets.red.b, 6);
  Serial.print(",\"blue_r\":");
  Serial.print(targets.blue.r, 6);
  Serial.print(",\"blue_g\":");
  Serial.print(targets.blue.g, 6);
  Serial.print(",\"blue_b\":");
  Serial.print(targets.blue.b, 6);
  Serial.print(",\"green_r\":");
  Serial.print(targets.green.r, 6);
  Serial.print(",\"green_g\":");
  Serial.print(targets.green.g, 6);
  Serial.print(",\"green_b\":");
  Serial.print(targets.green.b, 6);
  Serial.println("}");
}

// Prints ultrasonic calibration as a single JSON line.
void printUltraJson(const hardware::UltrasonicCal &cal) {
  Serial.print("{\"sensor\":\"ultrasonic\"");
  Serial.print(",\"slope\":");
  Serial.print(cal.slope, 6);
  Serial.print(",\"offset\":");
  Serial.print(cal.offset, 6);
  Serial.print(",\"min_cm\":");
  Serial.print(cal.min_cm);
  Serial.print(",\"max_cm\":");
  Serial.print(cal.max_cm);
  Serial.println("}");
}

// Prints IR profile reads for analysis and fusion tuning.
void printIrProfileJson(const IrProfile &p) {
  Serial.print("{\"sensor\":\"ir_profile\"");
  Serial.print(",\"cardboard_left\":");
  Serial.print(p.cardboard_left);
  Serial.print(",\"cardboard_right\":");
  Serial.print(p.cardboard_right);
  Serial.print(",\"red_left\":");
  Serial.print(p.red_left);
  Serial.print(",\"red_right\":");
  Serial.print(p.red_right);
  Serial.print(",\"blue_left\":");
  Serial.print(p.blue_left);
  Serial.print(",\"blue_right\":");
  Serial.print(p.blue_right);
  Serial.print(",\"green_left\":");
  Serial.print(p.green_left);
  Serial.print(",\"green_right\":");
  Serial.print(p.green_right);
  Serial.println("}");
}

// Starts IR profiling for material reflectance.
void startIrProfile() {
  g_state = CAL_IR_PROFILE_WAIT_CARDBOARD;
  Serial.println("IRP:STEP1 Place sensors over CARDBOARD, press ENTER (or type NEXT).");
  Serial.println("IRP:READY_CARDBOARD");
}

// Starts color calibration and prompts for white surface.
void startColorCal() {
  g_state = CAL_COLOR_WAIT_WHITE;
  Serial.println("COLOR:STEP1 Place sensor over WHITE reference, press ENTER (or type NEXT).");
  Serial.println("COLOR:READY_WHITE");
}

// Starts ultrasonic calibration and prompts for first distance.
void startUltraCal() {
  g_state = CAL_ULTRA_WAIT_POINT;
  g_ultra_index = 0;
  Serial.println("ULTRA:STEP1 Place a flat target at the shown distance.");
  Serial.print("ULTRA:READY_");
  Serial.println(kUltraPoints[g_ultra_index]);
  Serial.print("ULTRA:PLACE target at ");
  Serial.print(kUltraPoints[g_ultra_index]);
  Serial.println(" cm, press ENTER (or type NEXT).");
}

// Advances the current calibration step.
void handleNext() {
  if (g_state == CAL_IR_PROFILE_WAIT_CARDBOARD) {
    g_ir_profile_temp.cardboard_left =
        avgRaw(hardware::irReadLeftRaw, kIrSamples);
    g_ir_profile_temp.cardboard_right =
        avgRaw(hardware::irReadRightRaw, kIrSamples);
    g_state = CAL_IR_PROFILE_WAIT_RED;
    Serial.println("IRP:STEP2 Place sensors over RED, press ENTER (or type NEXT).");
    Serial.println("IRP:READY_RED");
    return;
  }

  if (g_state == CAL_IR_PROFILE_WAIT_RED) {
    g_ir_profile_temp.red_left = avgRaw(hardware::irReadLeftRaw, kIrSamples);
    g_ir_profile_temp.red_right = avgRaw(hardware::irReadRightRaw, kIrSamples);
    g_state = CAL_IR_PROFILE_WAIT_BLUE;
    Serial.println("IRP:STEP3 Place sensors over BLUE, press ENTER (or type NEXT).");
    Serial.println("IRP:READY_BLUE");
    return;
  }

  if (g_state == CAL_IR_PROFILE_WAIT_BLUE) {
    g_ir_profile_temp.blue_left = avgRaw(hardware::irReadLeftRaw, kIrSamples);
    g_ir_profile_temp.blue_right = avgRaw(hardware::irReadRightRaw, kIrSamples);
    g_state = CAL_IR_PROFILE_WAIT_GREEN;
    Serial.println("IRP:STEP4 Place sensors over GREEN, press ENTER (or type NEXT).");
    Serial.println("IRP:READY_GREEN");
    return;
  }

  if (g_state == CAL_IR_PROFILE_WAIT_GREEN) {
    g_ir_profile_temp.green_left = avgRaw(hardware::irReadLeftRaw, kIrSamples);
    g_ir_profile_temp.green_right = avgRaw(hardware::irReadRightRaw, kIrSamples);
    g_state = CAL_IDLE;
    printIrProfileJson(g_ir_profile_temp);
    return;
  }

  if (g_state == CAL_COLOR_WAIT_WHITE) {
    hardware::ColorCal cal = hardware::colorGetCal();
    cal.white = avgColorRaw(kColorSamples);
    hardware::colorSetCal(cal);

    g_state = CAL_COLOR_WAIT_BLACK;
    Serial.println("COLOR:STEP2 Place sensor over BLACK reference, press ENTER (or type NEXT).");
    Serial.println("COLOR:READY_BLACK");
    return;
  }

  if (g_state == CAL_COLOR_WAIT_BLACK) {
    hardware::ColorCal cal = hardware::colorGetCal();
    cal.black = avgColorRaw(kColorSamples);
    hardware::colorSetCal(cal);

    g_state = CAL_COLOR_WAIT_CARDBOARD;
    g_color_targets_temp.black.r = 0.0f;
    g_color_targets_temp.black.g = 0.0f;
    g_color_targets_temp.black.b = 0.0f;
    Serial.println(
        "COLOR:STEP3 Place sensor over CARDBOARD, press ENTER (or type NEXT).");
    Serial.println("COLOR:READY_CARDBOARD");
    return;
  }

  if (g_state == CAL_COLOR_WAIT_CARDBOARD) {
    g_color_targets_temp.cardboard = avgColorNorm(kColorSamples);
    g_state = CAL_COLOR_WAIT_RED;
    Serial.println("COLOR:STEP4 Place sensor over RED, press ENTER (or type NEXT).");
    Serial.println("COLOR:READY_RED");
    return;
  }

  if (g_state == CAL_COLOR_WAIT_RED) {
    g_color_targets_temp.red = avgColorNorm(kColorSamples);
    g_state = CAL_COLOR_WAIT_BLUE;
    Serial.println("COLOR:STEP5 Place sensor over BLUE, press ENTER (or type NEXT).");
    Serial.println("COLOR:READY_BLUE");
    return;
  }

  if (g_state == CAL_COLOR_WAIT_BLUE) {
    g_color_targets_temp.blue = avgColorNorm(kColorSamples);
    g_state = CAL_COLOR_WAIT_GREEN;
    Serial.println("COLOR:STEP6 Place sensor over GREEN, press ENTER (or type NEXT).");
    Serial.println("COLOR:READY_GREEN");
    return;
  }

  if (g_state == CAL_COLOR_WAIT_GREEN) {
    g_color_targets_temp.green = avgColorNorm(kColorSamples);
    hardware::colorSetTargets(g_color_targets_temp);
    hardware::ColorCal cal = hardware::colorGetCal();
    hardware::ColorTargets targets = hardware::colorGetTargets();
    g_state = CAL_IDLE;
    printColorJson(cal, targets);
    return;
  }

  if (g_state == CAL_ULTRA_WAIT_POINT) {
    g_ultra_raw[g_ultra_index] =
        hardware::ultrasonicReadRawMedian(kUltraSamples);
    ++g_ultra_index;

    if (g_ultra_index >= kUltraPointCount) {
      hardware::ultrasonicSetCalFromPairs(g_ultra_raw, kUltraPoints,
                                          kUltraPointCount);
      hardware::UltrasonicCal cal = hardware::ultrasonicGetCal();
      g_state = CAL_IDLE;
      printUltraJson(cal);
      return;
    }

    Serial.println("ULTRA:NEXT Place target at the next distance shown.");
    Serial.print("ULTRA:READY_");
    Serial.println(kUltraPoints[g_ultra_index]);
    Serial.print("ULTRA:PLACE target at ");
    Serial.print(kUltraPoints[g_ultra_index]);
    Serial.println(" cm, press ENTER (or type NEXT).");
    return;
  }
}

// Parses and applies calibration updates from SET_* commands.
bool handleSetCommand(const char *cmd, const char *args) {
  if (strcmp(cmd, "SET_IR") == 0) {
    int lw, lb, rw, rb, lt, rt;
    if (sscanf(args, "%d %d %d %d %d %d", &lw, &lb, &rw, &rb, &lt, &rt) != 6) {
      Serial.println("ERR:BAD_ARGS");
      return true;
    }

    hardware::IrCal cal;
    cal.left_white = lw;
    cal.left_black = lb;
    cal.right_white = rw;
    cal.right_black = rb;
    cal.left_thresh = lt;
    cal.right_thresh = rt;
    hardware::irSetCal(cal);
    Serial.println("OK:SET_IR");
    return true;
  }

  if (strcmp(cmd, "SET_COLOR") == 0) {
    int wr, wg, wb, br, bg, bb;
    if (sscanf(args, "%d %d %d %d %d %d", &wr, &wg, &wb, &br, &bg, &bb) != 6) {
      Serial.println("ERR:BAD_ARGS");
      return true;
    }

    hardware::ColorCal cal;
    cal.white.r = wr;
    cal.white.g = wg;
    cal.white.b = wb;
    cal.black.r = br;
    cal.black.g = bg;
    cal.black.b = bb;
    hardware::colorSetCal(cal);
    Serial.println("OK:SET_COLOR");
    return true;
  }

  if (strcmp(cmd, "SET_COLOR_TARGETS") == 0) {
    float cr, cg, cb, br, bg, bb;
    float rr, rg, rb, ur, ug, ub, gr, gg, gb;
    int parsed = sscanf(args, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
                        &cr, &cg, &cb, &br, &bg, &bb, &rr, &rg, &rb, &ur, &ug,
                        &ub, &gr, &gg, &gb);
    if (parsed != 15) {
      Serial.println("ERR:BAD_ARGS");
      return true;
    }

    hardware::ColorTargets targets;
    targets.cardboard.r = cr;
    targets.cardboard.g = cg;
    targets.cardboard.b = cb;
    targets.black.r = br;
    targets.black.g = bg;
    targets.black.b = bb;
    targets.red.r = rr;
    targets.red.g = rg;
    targets.red.b = rb;
    targets.blue.r = ur;
    targets.blue.g = ug;
    targets.blue.b = ub;
    targets.green.r = gr;
    targets.green.g = gg;
    targets.green.b = gb;
    hardware::colorSetTargets(targets);
    Serial.println("OK:SET_COLOR_TARGETS");
    return true;
  }

  if (strcmp(cmd, "SET_ULTRA") == 0) {
    float slope, offset;
    int min_cm, max_cm;
    if (sscanf(args, "%f %f %d %d", &slope, &offset, &min_cm, &max_cm) != 4) {
      Serial.println("ERR:BAD_ARGS");
      return true;
    }

    hardware::UltrasonicCal cal = hardware::ultrasonicGetCal();
    cal.slope = slope;
    cal.offset = offset;
    cal.min_cm = min_cm;
    cal.max_cm = max_cm;
    hardware::ultrasonicSetCal(cal);
    Serial.println("OK:SET_ULTRA");
    return true;
  }

  return false;
}

// Dispatches serial commands to calibration steps.
void handleCommand(char *cmd_line) {
  char *args = cmd_line;
  while (*args != '\0' && *args != ' ')
    ++args;
  if (*args == ' ') {
    *args = '\0';
    ++args;
  } else {
    args = cmd_line + strlen(cmd_line);
  }

  if (handleSetCommand(cmd_line, args))
    return;

  if (strcmp(cmd_line, "CAL_IR_PROFILE") == 0) {
    startIrProfile();
  } else if (strcmp(cmd_line, "CAL_COLOR") == 0) {
    startColorCal();
  } else if (strcmp(cmd_line, "CAL_ULTRA") == 0) {
    startUltraCal();
  } else if (strcmp(cmd_line, "NEXT") == 0) {
    handleNext();
  } else if (strcmp(cmd_line, "HELP") == 0) {
    Serial.println(
        "CMD: CAL_IR_PROFILE | CAL_COLOR | CAL_ULTRA | NEXT | SET_IR | "
        "SET_COLOR | SET_COLOR_TARGETS | SET_ULTRA");
  } else {
    Serial.println("ERR:UNKNOWN_CMD");
  }
}
} // namespace

namespace calibration_serial {
// Initializes Serial and prints a ready banner.
void init(long baud) {
  Serial.begin(baud > 0 ? baud : kDefaultBaud);
  Serial.println("CAL:READY");
}

// Polls Serial for commands and dispatches them.
void poll() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (g_cmd_len > 0) {
        g_cmd_buf[g_cmd_len] = '\0';
        handleCommand(g_cmd_buf);
        g_cmd_len = 0;
      }
      continue;
    }

    if (g_cmd_len < static_cast<int>(sizeof(g_cmd_buf)) - 1) {
      g_cmd_buf[g_cmd_len++] = c;
    }
  }
}
} // namespace calibration_serial
