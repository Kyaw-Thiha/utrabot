#pragma once
namespace telemetry {
void init();
void log(const char *msg);
void flush();
} // namespace telemetry
