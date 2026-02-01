#include "state_machine/states/obstacle_avoid.h"

#include "behavior/obstacle_avoid.h"
#include "behavior/line_follow.h"
#include "state_machine/state_machine.h"

namespace obstacle_avoid {
class ObstacleAvoidState : public BaseState {
public:
  void enter() override { behavior::obstacleAvoidInit(); }
  void update() override {
    behavior::obstacleAvoidUpdate();
    if (behavior::lineFollowIsOnLine()) {
      state_machine::transitionTo(StateId::LINE_FOLLOW);
    }
  }
  void exit() override {}
};

BaseState &instance() {
  static ObstacleAvoidState s;
  return s;
}
} // namespace obstacle_avoid
