#include "state_machine/state_machine.h"
#include "state_machine/base_state.h"

#include "state_machine/states/find_black.h"
#include "state_machine/states/line_follow.h"
#include "state_machine/states/obstacle_avoid.h"
#include "state_machine/states/obstacle_course.h"
#include "state_machine/states/push_cube.h"
#include "state_machine/states/ramp_climb.h"
#include "state_machine/states/return_home.h"

namespace state_machine {
static BaseState *current = nullptr;

void init() {
  current = &line_follow::instance();
  current->enter();
}

void update() {
  if (current) {
    current->update();
  }
}

void transitionTo(int next_id) {
  if (current)
    current->exit();
  switch (next_id) {
  case StateId::RAMP_CLIMB:
    current = &ramp_climb::instance();
    break;
  case StateId::FIND_BLACK:
    current = &find_black::instance();
    break;
  case StateId::PUSH_CUBE:
    current = &push_cube::instance();
    break;
  case StateId::OBSTACLE_COURSE:
    current = &obstacle_course::instance();
    break;
  case StateId::RETURN_HOME:
    current = &return_home::instance();
    break;
  case StateId::LINE_FOLLOW:
    current = &line_follow::instance();
    break;
  case StateId::OBSTACLE_AVOID:
    current = &obstacle_avoid::instance();
    break;
  default:
    current = &ramp_climb::instance();
    break;
  }
  current->enter();
}
} // namespace state_machine
