#include "hardware/motor_driver.h"

#include <Arduino.h>

#include "config.h"
#include "pins.h"

namespace hardware {
namespace {
int clampPwm(int value) {
  if (value > 255) {
    return 255;
  }
  if (value < -255) {
    return -255;
  }
  return value;
}

void setMotor(int in1, int in2, int en, int pwm) {
  int clamped = clampPwm(pwm);
  if (clamped > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(en, clamped);
  } else if (clamped < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(en, -clamped);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    analogWrite(en, 0);
  }
}
} // namespace

void motorInit() {
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(MOTOR_A_EN, OUTPUT);

  pinMode(MOTOR_B_IN1, OUTPUT);
  pinMode(MOTOR_B_IN2, OUTPUT);
  pinMode(MOTOR_B_EN, OUTPUT);

  motorStop();
}

void motorSet(int left_pwm, int right_pwm) {
  int left = config::invertLeftMotor() ? -left_pwm : left_pwm;
  int right = config::invertRightMotor() ? -right_pwm : right_pwm;
  setMotor(MOTOR_A_IN1, MOTOR_A_IN2, MOTOR_A_EN, left);
  setMotor(MOTOR_B_IN1, MOTOR_B_IN2, MOTOR_B_EN, right);
}

void motorStop() {
  setMotor(MOTOR_A_IN1, MOTOR_A_IN2, MOTOR_A_EN, 0);
  setMotor(MOTOR_B_IN1, MOTOR_B_IN2, MOTOR_B_EN, 0);
}
} // namespace hardware
