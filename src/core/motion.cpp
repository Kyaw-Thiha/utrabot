#include "core/motion.h"
#include "hardware/motor_driver.h"

namespace motion {
void init() { hardware::motorInit(); }
void drive(int speed) { hardware::motorSet(speed, speed); }
void turn(int speed) { hardware::motorSet(speed, -speed); }
void stop() { hardware::motorStop(); }
} // namespace motion
