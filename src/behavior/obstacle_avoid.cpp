#include "behavior/obstacle_avoid.h"

#include <Arduino.h>

#include "behavior/line_follow.h"
#include "hardware/motor_driver.h"
#include "hardware/ultrasonic.h"

namespace behavior {
namespace {
enum Phase {
  PHASE_IDLE = 0,
  PHASE_TURN_LEFT_1,
  PHASE_DRIVE_LEFT_CLEAR,
  PHASE_TURN_RIGHT_1,
  PHASE_DRIVE_FORWARD_PASS,
  PHASE_TURN_RIGHT_2,
  PHASE_DRIVE_RIGHT_CLEAR,
  PHASE_TURN_LEFT_2,
  PHASE_REACQUIRE
};

ObstacleAvoidConfig g_cfg = {
    20,   // avoidance_threshold_cm
    20,   // max_obstacle_width_cm
    5,    // clearance_cm
    600,  // turn_90_ms
    0.05f, // cm_per_ms
    140,  // drive_speed
    120   // turn_speed
};

Phase g_phase = PHASE_IDLE;
unsigned long g_phase_start = 0;
unsigned long g_phase_duration = 0;

unsigned long cmToMs(int cm) {
  if (g_cfg.cm_per_ms <= 0.0f)
    return 0;
  return static_cast<unsigned long>(cm / g_cfg.cm_per_ms);
}

void startPhase(Phase p, unsigned long duration_ms) {
  g_phase = p;
  g_phase_start = millis();
  g_phase_duration = duration_ms;
}

bool phaseDone() {
  return (millis() - g_phase_start) >= g_phase_duration;
}

void driveForward() {
  hardware::motorSet(g_cfg.drive_speed, g_cfg.drive_speed);
}

void driveTurnLeft() {
  hardware::motorSet(-g_cfg.turn_speed, g_cfg.turn_speed);
}

void driveTurnRight() {
  hardware::motorSet(g_cfg.turn_speed, -g_cfg.turn_speed);
}
} // namespace

void obstacleAvoidInit() { g_phase = PHASE_IDLE; }

bool obstacleDetected() {
  int cm = hardware::ultrasonicReadCm();
  return (cm > 0) && (cm <= g_cfg.avoidance_threshold_cm);
}

void obstacleAvoidUpdate() {
  switch (g_phase) {
  case PHASE_IDLE:
    startPhase(PHASE_TURN_LEFT_1, g_cfg.turn_90_ms);
    break;
  case PHASE_TURN_LEFT_1:
    driveTurnLeft();
    if (phaseDone())
      startPhase(PHASE_DRIVE_LEFT_CLEAR,
                 cmToMs(g_cfg.max_obstacle_width_cm + g_cfg.clearance_cm));
    break;
  case PHASE_DRIVE_LEFT_CLEAR:
    driveForward();
    if (phaseDone())
      startPhase(PHASE_TURN_RIGHT_1, g_cfg.turn_90_ms);
    break;
  case PHASE_TURN_RIGHT_1:
    driveTurnRight();
    if (phaseDone())
      startPhase(PHASE_DRIVE_FORWARD_PASS,
                 cmToMs(g_cfg.max_obstacle_width_cm + g_cfg.clearance_cm));
    break;
  case PHASE_DRIVE_FORWARD_PASS:
    driveForward();
    if (phaseDone())
      startPhase(PHASE_TURN_RIGHT_2, g_cfg.turn_90_ms);
    break;
  case PHASE_TURN_RIGHT_2:
    driveTurnRight();
    if (phaseDone())
      startPhase(PHASE_DRIVE_RIGHT_CLEAR,
                 cmToMs(g_cfg.max_obstacle_width_cm + g_cfg.clearance_cm));
    break;
  case PHASE_DRIVE_RIGHT_CLEAR:
    driveForward();
    if (phaseDone())
      startPhase(PHASE_TURN_LEFT_2, g_cfg.turn_90_ms);
    break;
  case PHASE_TURN_LEFT_2:
    driveTurnLeft();
    if (phaseDone())
      startPhase(PHASE_REACQUIRE, 0);
    break;
  case PHASE_REACQUIRE:
    hardware::motorSet(g_cfg.drive_speed / 2, g_cfg.drive_speed / 2);
    if (lineFollowIsOnLine()) {
      hardware::motorStop();
      g_phase = PHASE_IDLE;
    }
    break;
  }
}

void obstacleAvoidSetConfig(const ObstacleAvoidConfig &cfg) { g_cfg = cfg; }
ObstacleAvoidConfig obstacleAvoidGetConfig() { return g_cfg; }

} // namespace behavior
