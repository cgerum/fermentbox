#include "status.h"

#include <stdio.h>

static FermentboxStatus status;

static const int MESSAGE_BUFFER_SIZE = 160;
static char statusMessageBuffer[MESSAGE_BUFFER_SIZE];

struct StatusMessage {
  FermentboxStatus status;
  const char *code;
  const char *message;
  bool affects_ok;
};

static const StatusMessage messages[] = {
    {STATUS_SENSOR_FAILED, "sensor_failed", "Temperature/Humidity sensor failed",
     true},
    {STATUS_NETWORK_UNAVAILABLE, "network_unavailable",
     "Network connection unavailable", true},
    {STATUS_CONFIG_MISSING, "config_missing", "Wi-Fi configuration missing",
     true},
    {STATUS_SCHEDULE_INACTIVE, "schedule_inactive", "No schedule running",
     false},
    {STATUS_CONTROL_IDLE, "control_idle", "Control loop is idle", false},
};

FermentboxStatus get_status() { return status; }

const char *get_status_message(FermentboxStatus stat) {
  if (stat == STATUS_NORMAL) {
    return "Normal";
  }

  int offset = 0;
  for (auto currentMessage : messages) {
    if (!(stat & currentMessage.status)) {
      continue;
    }

    int written = snprintf(statusMessageBuffer + offset,
                           MESSAGE_BUFFER_SIZE - offset, "%s%s",
                           offset == 0 ? "" : "; ", currentMessage.message);
    if (written < 0 || written >= MESSAGE_BUFFER_SIZE - offset) {
      break;
    }
    offset += written;
  }

  if (offset == 0) {
    return "Unknown error";
  }

  return statusMessageBuffer;
}

const char *get_status_message() { return get_status_message(status); }

const char *get_status_code(FermentboxStatus stat) {
  for (auto currentMessage : messages) {
    if (stat & currentMessage.status) {
      return currentMessage.code;
    }
  }

  return "normal";
}

bool status_is_ok(FermentboxStatus stat) {
  for (auto currentMessage : messages) {
    if ((stat & currentMessage.status) && currentMessage.affects_ok) {
      return false;
    }
  }
  return true;
}

void set_status(FermentboxStatus stat) { status = stat; }

void add_status(FermentboxStatus stat) {
  status = static_cast<FermentboxStatus>(status | stat);
}

void clear_status(FermentboxStatus stat) {
  status = static_cast<FermentboxStatus>(status & ~stat);
}