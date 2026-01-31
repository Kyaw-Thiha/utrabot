#include "state_machine/states/ramp_climb.h"

namespace ramp_climb {
class RampClimbState : public BaseState {
public:
  void enter() override {}
  void update() override {}
  void exit() override {}
};

BaseState &instance() {
  static RampClimbState s;
  return s;
}
} // namespace ramp_climb
