#pragma once

enum FermentboxStatus {
  STATUS_NORMAL = 0,
  STATUS_SENSOR_FAILED = 1 << 0,
  STATUS_NETWORK_UNAVAILABLE = 1 << 1,
  STATUS_CONFIG_MISSING = 1 << 2,
  STATUS_SCHEDULE_INACTIVE = 1 << 3,
  STATUS_CONTROL_IDLE = 1 << 4,
};

FermentboxStatus get_status();
const char *get_status_message();
const char *get_status_message(FermentboxStatus stat);
const char *get_status_code(FermentboxStatus stat);
bool status_is_ok(FermentboxStatus stat);
void set_status(FermentboxStatus stat);
void add_status(FermentboxStatus stat);
void clear_status(FermentboxStatus stat);