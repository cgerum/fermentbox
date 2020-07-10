#pragma once

enum FermentboxStatus { STATUS_NORMAL = 0, STATUS_SENSOR_FAILED = 1 };
FermentboxStatus get_status();
const char *get_status_message();
void set_status(FermentboxStatus stat);