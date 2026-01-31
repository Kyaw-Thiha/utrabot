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
  CAL_IR_WAIT_WHITE,
  CAL_IR_WAIT_BLACK,
  CAL_COLOR_WAIT_WHITE,
  CAL_COLOR_WAIT_BLACK,
  CAL_ULTRA_WAIT_POINT
};

CalState g_state = CAL_IDLE;
int g_ultra_index = 0;
int g_ultra_raw[kUltraPointCount];

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
void printColorJson(const hardware::ColorCal &cal) {
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

// Starts IR calibration and prompts for white surface.
void startIrCal() {
  g_state = CAL_IR_WAIT_WHITE;
  Serial.println("IR:READY_WHITE");
}

// Starts color calibration and prompts for white surface.
void startColorCal() {
  g_state = CAL_COLOR_WAIT_WHITE;
  Serial.println("COLOR:READY_WHITE");
}

// Starts ultrasonic calibration and prompts for first distance.
void startUltraCal() {
  g_state = CAL_ULTRA_WAIT_POINT;
  g_ultra_index = 0;
  Serial.print("ULTRA:READY_");
  Serial.println(kUltraPoints[g_ultra_index]);
}

// Advances the current calibration step.
void handleNext() {
  if (g_state == CAL_IR_WAIT_WHITE) {
    hardware::IrCal cal = hardware::irGetCal();
    cal.left_white = avgRaw(hardware::irReadLeftRaw, kIrSamples);
    cal.right_white = avgRaw(hardware::irReadRightRaw, kIrSamples);
    hardware::irSetCal(cal);

    g_state = CAL_IR_WAIT_BLACK;
    Serial.println("IR:READY_BLACK");
    return;
  }

  if (g_state == CAL_IR_WAIT_BLACK) {
    hardware::IrCal cal = hardware::irGetCal();
    cal.left_black = avgRaw(hardware::irReadLeftRaw, kIrSamples);
    cal.right_black = avgRaw(hardware::irReadRightRaw, kIrSamples);
    cal.left_thresh = (cal.left_white + cal.left_black) / 2;
    cal.right_thresh = (cal.right_white + cal.right_black) / 2;
    hardware::irSetCal(cal);

    g_state = CAL_IDLE;
    printIrJson(cal);
    return;
  }

  if (g_state == CAL_COLOR_WAIT_WHITE) {
    hardware::ColorCal cal = hardware::colorGetCal();
    cal.white = avgColorRaw(kColorSamples);
    hardware::colorSetCal(cal);

    g_state = CAL_COLOR_WAIT_BLACK;
    Serial.println("COLOR:READY_BLACK");
    return;
  }

  if (g_state == CAL_COLOR_WAIT_BLACK) {
    hardware::ColorCal cal = hardware::colorGetCal();
    cal.black = avgColorRaw(kColorSamples);
    hardware::colorSetCal(cal);

    g_state = CAL_IDLE;
    printColorJson(cal);
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

    Serial.print("ULTRA:READY_");
    Serial.println(kUltraPoints[g_ultra_index]);
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

  if (strcmp(cmd_line, "CAL_IR") == 0) {
    startIrCal();
  } else if (strcmp(cmd_line, "CAL_COLOR") == 0) {
    startColorCal();
  } else if (strcmp(cmd_line, "CAL_ULTRA") == 0) {
    startUltraCal();
  } else if (strcmp(cmd_line, "NEXT") == 0) {
    handleNext();
  } else if (strcmp(cmd_line, "HELP") == 0) {
    Serial.println(
        "CMD: CAL_IR | CAL_COLOR | CAL_ULTRA | NEXT | SET_IR | SET_COLOR | SET_ULTRA");
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
