#pragma once

namespace behavior {

struct ObstacleAvoidConfig {
  int avoidance_threshold_cm;
  int max_obstacle_width_cm;
  int clearance_cm;
  int turn_90_ms;
  float cm_per_ms;
  int drive_speed;
  int turn_speed;
};

void obstacleAvoidInit();
void obstacleAvoidUpdate();

void obstacleAvoidSetConfig(const ObstacleAvoidConfig &cfg);
ObstacleAvoidConfig obstacleAvoidGetConfig();

bool obstacleDetected();

} // namespace behavior
