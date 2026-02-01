#include "behavior/line_follow.h"

#include <math.h>

#include "behavior/obstacle_avoid.h"
#include "hardware/color_sensor.h"
#include "hardware/ir_sensor.h"
#include "hardware/motor_driver.h"
#include "state_machine/base_state.h"
#include "state_machine/state_machine.h"

namespace behavior {
namespace {
LineFollowConfig g_cfg = {
    140,  // base_speed
    110,  // turn_speed
    0.18f, // match_thresh
    0.05f, // bg_margin
    static_cast<uint8_t>((1 << LINE_BLACK) | (1 << LINE_RED))};

float colorDist(const hardware::ColorNorm &n, const hardware::ColorTarget &t) {
  float dr = n.r - t.r;
  float dg = n.g - t.g;
  float db = n.b - t.b;
  return sqrtf(dr * dr + dg * dg + db * db);
}

void getAllowedTargets(const hardware::ColorTargets &targets,
                       const hardware::ColorTarget *&list, int &count) {
  static hardware::ColorTarget buf[LINE_COLOR_COUNT];
  int idx = 0;

  if (g_cfg.allowed_mask & (1 << LINE_BLACK))
    buf[idx++] = targets.black;
  if (g_cfg.allowed_mask & (1 << LINE_RED))
    buf[idx++] = targets.red;
  if (g_cfg.allowed_mask & (1 << LINE_GREEN))
    buf[idx++] = targets.green;
  if (g_cfg.allowed_mask & (1 << LINE_BLUE))
    buf[idx++] = targets.blue;

  list = buf;
  count = idx;
}

bool isOnLine(const hardware::ColorNorm &n) {
  hardware::ColorTargets targets = hardware::colorGetTargets();
  const hardware::ColorTarget *allowed = nullptr;
  int allowed_count = 0;

  getAllowedTargets(targets, allowed, allowed_count);
  if (allowed_count == 0)
    return false;

  float best = 999.0f;
  for (int i = 0; i < allowed_count; ++i) {
    float d = colorDist(n, allowed[i]);
    if (d < best)
      best = d;
  }

  hardware::ColorTarget white_target;
  white_target.r = 1.0f;
  white_target.g = 1.0f;
  white_target.b = 1.0f;

  float bg_cardboard = colorDist(n, targets.cardboard);
  float bg_white = colorDist(n, white_target);
  float bg_best = (bg_cardboard < bg_white) ? bg_cardboard : bg_white;

  if (best > g_cfg.match_thresh)
    return false;

  return (best + g_cfg.bg_margin) < bg_best;
}

void driveSearch() {
  int left = hardware::irReadLeftRaw();
  int right = hardware::irReadRightRaw();

  if (left && !right) {
    hardware::motorSet(-g_cfg.turn_speed, g_cfg.turn_speed);
  } else if (right && !left) {
    hardware::motorSet(g_cfg.turn_speed, -g_cfg.turn_speed);
  } else {
    hardware::motorSet(g_cfg.turn_speed, -g_cfg.turn_speed);
  }
}
} // namespace

void lineFollowInit() {}

void lineFollowUpdate() {
  if (obstacleDetected()) {
    state_machine::transitionTo(StateId::OBSTACLE_AVOID);
    return;
  }

  hardware::ColorNorm n = hardware::colorReadNormalized();

  if (isOnLine(n)) {
    hardware::motorSet(g_cfg.base_speed, g_cfg.base_speed);
  } else {
    driveSearch();
  }
}

bool lineFollowIsOnLine() {
  hardware::ColorNorm n = hardware::colorReadNormalized();
  return isOnLine(n);
}

void lineFollowSetConfig(const LineFollowConfig &cfg) { g_cfg = cfg; }
LineFollowConfig lineFollowGetConfig() { return g_cfg; }

void lineFollowSetAllowedMask(uint8_t mask) { g_cfg.allowed_mask = mask; }
uint8_t lineFollowGetAllowedMask() { return g_cfg.allowed_mask; }

} // namespace behavior
