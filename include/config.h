#pragma once

#include <stdint.h>

#include "behavior/line_follow.h"
#include "behavior/obstacle_avoid.h"

namespace config {

inline behavior::LineFollowConfig lineFollowConfig() {
  behavior::LineFollowConfig cfg;
  cfg.base_speed = 140;
  cfg.turn_speed = 110;
  cfg.match_thresh = 0.18f;
  cfg.bg_margin = 0.05f;
  cfg.allowed_mask =
      static_cast<uint8_t>((1 << behavior::LINE_BLACK) |
                           (1 << behavior::LINE_RED));
  return cfg;
}

inline behavior::ObstacleAvoidConfig obstacleAvoidConfig() {
  behavior::ObstacleAvoidConfig cfg;
  cfg.avoidance_threshold_cm = 20;
  cfg.max_obstacle_width_cm = 20;
  cfg.clearance_cm = 5;
  cfg.turn_90_ms = 600;
  cfg.cm_per_ms = 0.05f;
  cfg.drive_speed = 140;
  cfg.turn_speed = 120;
  return cfg;
}

} // namespace config
