#include <Arduino.h>

#include "core/system/calibration_serial.h"
#include "hardware/color_sensor.h"
#include "hardware/ir_sensor.h"
#include "hardware/motor_driver.h"
#include "hardware/ultrasonic.h"
#include "state_machine/state_machine.h"

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  hardware::ultrasonicInit();
  hardware::irInit();
  hardware::colorInit();
  hardware::motorInit();
  calibration_serial::init(115200);
  state_machine::init();
}

void loop() {
  calibration_serial::poll();
  state_machine::update();
}
