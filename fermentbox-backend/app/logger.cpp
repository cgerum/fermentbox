#include <Data/Stream/FileStream.h>
#include <SmingCore.h>

#include "control_loop.h"
#include "sensors.h"

#include "logger.h"

#define LOG_FILE_SIZE (1024 * 8 * 16)
#define LOG_BUFFER_SIZE 8
#define LOG_INTERVALL 30
#define LOG_FILE "fermentbox.log"
#define LOG_FILE_OLD "fermentbox.log.old"
#define LOG_SCALE_FACTOR 256.0f // 8 Bit for Fractional part

#define TEMP_CONTROL_ACTIVE 1
#define HUMIDITY_CONTROL_ACTIVE 2

struct LogEntry {
  uint32_t timestamp;
  int16_t current_temp;
  int16_t target_temp;
  int16_t current_humidity;
  int16_t target_humidity;
  uint8_t flags;
};

class Logger {
public:
  Logger() : stream() { local_pos = 0; }

  void do_log() {
    LogEntry &current_entry = local_buffer[local_pos];

    auto &sensors = getSensors();
    auto &current_measurement = sensors.getLastMeasurement();
    auto control_state = getControlState();

    current_entry.timestamp = current_measurement.date.toUnixTime();
    current_entry.current_temp =
        current_measurement.temperature * LOG_SCALE_FACTOR;
    current_entry.current_humidity =
        current_measurement.humidity * LOG_SCALE_FACTOR;

    current_entry.target_temp =
        control_state.target_temperature * LOG_SCALE_FACTOR;
    current_entry.target_humidity =
        control_state.target_humidity * LOG_SCALE_FACTOR;

    current_entry.flags = 0;
    if (control_state.temperature_active) {
      current_entry.flags |= TEMP_CONTROL_ACTIVE;
    }
    if (control_state.humidity_active) {
      current_entry.flags |= HUMIDITY_CONTROL_ACTIVE;
    }

    local_pos += 1;
    if (local_pos >= LOG_BUFFER_SIZE) {

      // debugf("Writing result size: %d\n", stream.getSize());

      stream.write((uint8_t *)&(local_buffer[0]), sizeof(local_buffer));

      if (stream.getSize() >= LOG_FILE_SIZE) {
        stream.close();
        if (fileExist(LOG_FILE_OLD)) {
          debugf("Deleting old file");
          fileDelete(LOG_FILE_OLD);
        }
        fileRename(LOG_FILE, LOG_FILE_OLD);
        stream.open(LOG_FILE, IFS::OpenFlag::Read | IFS::OpenFlag::Write |
                                  IFS::OpenFlag::Create);
      }

      local_pos = 0;

      // TODO: dump to file
    }
  }

  void run() {
    stream.open(LOG_FILE, IFS::OpenFlag::Read | IFS::OpenFlag::Write |
                              IFS::OpenFlag::Create);
    stream.seek(stream.getSize());
    if (!stream.isValid()) {
      debugf("Could not initialize logging\n");
      return;
    }

    // debugf("Logfile: %s sized: %d \n", LOG_FILE, stream.getSize());

    timer
        .initializeMs(1000 * LOG_INTERVALL,
                      TimerDelegate(&Logger::do_log, this))
        .start();
  }

private:
  FileStream stream;

  LogEntry local_buffer[LOG_BUFFER_SIZE];
  int16_t local_pos;
  Timer timer;
};

static Logger logger;

void startLogger() { logger.run(); }