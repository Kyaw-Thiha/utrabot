#include "state_machine/states/push_cube.h"

namespace push_cube {
class PushCubeState : public BaseState {
public:
  void enter() override {}
  void update() override {}
  void exit() override {}
};

BaseState &instance() {
  static PushCubeState s;
  return s;
}
} // namespace push_cube
