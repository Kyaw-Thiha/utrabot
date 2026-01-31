#include "state_machine/states/return_home.h"

namespace return_home {
class ReturnHomeState : public BaseState {
public:
  void enter() override {}
  void update() override {}
  void exit() override {}
};

BaseState &instance() {
  static ReturnHomeState s;
  return s;
}
} // namespace return_home
