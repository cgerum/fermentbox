#include "status.h"

#include <stdio.h>

static FermentboxStatus status;

static const int MESSAGE_BUFFER_SIZE = 160;
static char statusMessageBuffer[MESSAGE_BUFFER_SIZE];

struct StatusMessage {
  FermentboxStatus status;
  const char *message;
};

static const StatusMessage messages[] = {
    {STATUS_SENSOR_FAILED, "Temperature/Humidity Sensor failed"}};

FermentboxStatus get_status() { return status; }

const char *get_status_message() {
  if (status == STATUS_NORMAL) {
    return "Normal";
  }

  int offset = 0;
  for (auto currentMessage : messages) {
    if (!(status & currentMessage.status)) {
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

void set_status(FermentboxStatus stat) { status = stat; }

void add_status(FermentboxStatus stat) {
  status = static_cast<FermentboxStatus>(status | stat);
}

void clear_status(FermentboxStatus stat) {
  status = static_cast<FermentboxStatus>(status & ~stat);
}