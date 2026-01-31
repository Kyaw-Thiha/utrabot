#include "state_machine/states/find_black.h"

namespace find_black {
class FindBlackState : public BaseState {
public:
  void enter() override {}
  void update() override {}
  void exit() override {}
};

BaseState &instance() {
  static FindBlackState s;
  return s;
}
} // namespace find_black
