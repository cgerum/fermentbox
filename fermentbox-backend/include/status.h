#pragma once

enum FermentboxStatus {
  STATUS_NORMAL = 0,
  STATUS_SENSOR_FAILED = 1 << 0,
};

FermentboxStatus get_status();
const char *get_status_message();
void set_status(FermentboxStatus stat);
void add_status(FermentboxStatus stat);
void clear_status(FermentboxStatus stat);