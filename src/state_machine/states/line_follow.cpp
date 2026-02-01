#include "state_machine/states/line_follow.h"

#include "behavior/line_follow.h"

namespace line_follow {
class LineFollowState : public BaseState {
public:
  void enter() override { behavior::lineFollowInit(); }
  void update() override { behavior::lineFollowUpdate(); }
  void exit() override {}
};

BaseState &instance() {
  static LineFollowState s;
  return s;
}
} // namespace line_follow
