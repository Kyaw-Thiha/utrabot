#pragma once
namespace hardware {
void motorInit();
void motorSet(int left_pwm, int right_pwm);
void motorStop();
} // namespace hardware
