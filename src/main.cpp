#include <Arduino.h>

#include "task_select.h"

#include "core/system/calibration_serial.h"
#include "hardware/color_sensor.h"
#include "hardware/ir_sensor.h"
#include "hardware/motor_driver.h"
#include "hardware/ultrasonic.h"
#include "state_machine/state_machine.h"

void setup() {
#if ACTIVE_TASK == TASK_CALIBRATION
  hardware::ultrasonicInit();
  hardware::irInit();
  hardware::colorInit();
  calibration_serial::init(115200);
#elif ACTIVE_TASK == TASK_LINE_FOLLOW
  hardware::ultrasonicInit();
  hardware::irInit();
  hardware::colorInit();
  hardware::motorInit();
  calibration_serial::init(115200);
  state_machine::init();
#endif
}

void loop() {
#if ACTIVE_TASK == TASK_CALIBRATION
  calibration_serial::poll();
#elif ACTIVE_TASK == TASK_LINE_FOLLOW
  calibration_serial::poll();
  state_machine::update();
#endif
}
