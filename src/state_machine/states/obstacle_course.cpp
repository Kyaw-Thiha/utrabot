#include "state_machine/states/obstacle_course.h"

namespace obstacle_course {
class ObstacleCourseState : public BaseState {
public:
  void enter() override {}
  void update() override {}
  void exit() override {}
};

BaseState &instance() {
  static ObstacleCourseState s;
  return s;
}
} // namespace obstacle_course
