#pragma once

enum StateId {
  RAMP_CLIMB = 0,
  FIND_BLACK = 1,
  PUSH_CUBE = 2,
  OBSTACLE_COURSE = 3,
  RETURN_HOME = 4,
  LINE_FOLLOW = 5,
  OBSTACLE_AVOID = 6
};

class BaseState {
public:
  virtual ~BaseState() {}
  virtual void enter() = 0;
  virtual void update() = 0;
  virtual void exit() = 0;
};
