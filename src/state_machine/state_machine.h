#pragma once

namespace state_machine {
void init();
void update();
void transitionTo(int next_id);
} // namespace state_machine
