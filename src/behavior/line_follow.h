#pragma once

#include <stdint.h>

namespace behavior {

enum LineColor {
  LINE_BLACK = 0,
  LINE_RED = 1,
  LINE_GREEN = 2,
  LINE_BLUE = 3,
  LINE_COLOR_COUNT = 4
};

struct LineFollowConfig {
  int base_speed;
  int turn_speed;
  float match_thresh;
  float bg_margin;
  uint8_t allowed_mask;
};

void lineFollowInit();
void lineFollowUpdate();

void lineFollowSetConfig(const LineFollowConfig &cfg);
LineFollowConfig lineFollowGetConfig();

void lineFollowSetAllowedMask(uint8_t mask);
uint8_t lineFollowGetAllowedMask();

} // namespace behavior
