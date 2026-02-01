#include "hardware/servo.h"

#include <Arduino.h>
#include <Servo.h>

#include "pins.h"

namespace hardware {
namespace {
Servo g_servo;

int clampAngle(int angle_deg) {
  if (angle_deg < 0) {
    return 0;
  }
  if (angle_deg > 180) {
    return 180;
  }
  return angle_deg;
}
} // namespace

void servoInit() {
  g_servo.attach(SERVO_PIN);
  servoSetAngle(90);
}

void servoSetAngle(int angle_deg) {
  int clamped = clampAngle(angle_deg);
  g_servo.write(clamped);
}
} // namespace hardware
