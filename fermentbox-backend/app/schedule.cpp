#include <ArduinoJson.h>

#include "schedule.h"

// Maximum schedule size is 30 KB
#define ScheduleJsonBufferSize (1024 * 20)

static StaticJsonDocument<ScheduleJsonBufferSize> scheduleDoc;

struct ScheduleItem {
  int32_t duration;

  bool temperature_active;
  float temperature_start;
  float temperature_end;

  bool humidity_active;
  float start_humidity;
  float end_humidity;
};

class Schedule {

public:
  Schedule() {
    size = 0;
    active = false;
    pos = 0;
    offset = 0;
  }

  void load(String &filename) {
    if (Json::loadFromFile(scheduleDoc, filename)) {
      auto array = scheduleDoc.as<JsonArray>();
      for (JsonVariant v : array) {
        const auto object = v.as<JsonObject>();
        auto duration = object.getMember("duration").as<int>();
        debugf("duration: %i\n", duration);
      }
    }
  }

private:
  ScheduleItem items[20];
  int32_t size;
  int32_t pos;    // Currently active item
  int32_t offset; // Offset inside current item in seconds;
  bool active;
  Timer timer;
};

Schedule currentSchedule;

void startSchedule(String &name) {
  String filename = getScheduleFileName(name);
  currentSchedule.load(filename);
}

void stopSchedule() {}