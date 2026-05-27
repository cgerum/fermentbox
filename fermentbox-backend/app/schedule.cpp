#include <ArduinoJson.h>

#include "control_loop.h"
#include "schedule.h"

// Maximum schedule size is 30 KB
#define ScheduleJsonBufferSize (1024 * 1)
#define MaxScheduleItems 20
#define SecondsPerHour 3600.0f

static StaticJsonDocument<ScheduleJsonBufferSize> scheduleDoc;

struct ScheduleItem {
  int32_t duration_seconds;

  bool temperature_active;
  float temperature_start;
  float temperature_end;

  bool humidity_active;
  float humidity_start;
  float humidity_end;
};

class Schedule {

public:
  Schedule() {
    size = 0;
    active = false;
    pos = 0;
    offset = 0;
    has_snapshot = false;
  }

  bool load(String &filename, String *errorMessage) {
    size = 0;
    pos = 0;
    offset = 0;

    if (!Json::loadFromFile(scheduleDoc, filename)) {
      setError(errorMessage, "Could not parse schedule JSON");
      return false;
    }

    if (!scheduleDoc.is<JsonArray>()) {
      setError(errorMessage, "Schedule root must be an array");
      return false;
    }

    JsonArray array = scheduleDoc.as<JsonArray>();
    if (array.size() <= 0) {
      setError(errorMessage, "Schedule must contain at least one step");
      return false;
    }
    if (array.size() > MaxScheduleItems) {
      setError(errorMessage, "Schedule has too many steps");
      return false;
    }

    for (JsonVariant v : array) {
      if (!v.is<JsonObject>()) {
        setError(errorMessage, "Schedule step must be an object");
        return false;
      }

      const JsonObject object = v.as<JsonObject>();
      ScheduleItem &item = items[size];
      if (!parseItem(object, item, errorMessage)) {
        return false;
      }
      size += 1;
    }

    return true;
  }

  bool start(String *errorMessage) {
    if (size <= 0) {
      setError(errorMessage, "Schedule has no executable steps");
      return false;
    }

    snapshot = getControlState();
    has_snapshot = true;

    pos = 0;
    offset = 0;
    active = true;
    applyCurrentStep();
    timer.initializeMs(1000, TimerDelegate(&Schedule::onTick, this)).start();
    debugf("Schedule started with %d step(s)", size);
    return true;
  }

  bool isActive() const { return active; }

  void stop(bool restoreControlSnapshot) {
    timer.stop();
    if (!active) {
      return;
    }

    active = false;
    if (restoreControlSnapshot && has_snapshot) {
      setControlState(snapshot);
      debugf("Schedule stopped and control targets restored");
    } else {
      debugf("Schedule stopped");
    }
  }

private:
  static void setError(String *errorMessage, const String &message) {
    if (errorMessage != nullptr) {
      *errorMessage = message;
    }
    debugf("Schedule error: %s", message.c_str());
  }

  bool parseItem(const JsonObject &object, ScheduleItem &item,
                 String *errorMessage) {
    JsonVariant duration_variant = object["duration"];
    if (duration_variant.isNull() || !duration_variant.is<float>()) {
      setError(errorMessage, "Step duration is required");
      return false;
    }
    float duration_hours = duration_variant.as<float>();
    if (duration_hours <= 0.0f) {
      setError(errorMessage, "Step duration must be greater than zero");
      return false;
    }

    item.duration_seconds = (int32_t)(duration_hours * SecondsPerHour);
    if (item.duration_seconds <= 0) {
      item.duration_seconds = 1;
    }

    JsonVariant temperature_active_variant = object["temperature_active"];
    JsonVariant humidity_active_variant = object["humidity_active"];
    if (temperature_active_variant.isNull() || humidity_active_variant.isNull() ||
        !temperature_active_variant.is<bool>() ||
        !humidity_active_variant.is<bool>()) {
      setError(errorMessage, "temperature_active and humidity_active required");
      return false;
    }

    item.temperature_active = temperature_active_variant.as<bool>();
    if (item.temperature_active) {
      JsonVariant temperature_start_variant = object["temperature_start"];
      if (temperature_start_variant.isNull() ||
          !temperature_start_variant.is<float>()) {
        setError(errorMessage, "temperature_start required when active");
        return false;
      }
      item.temperature_start = temperature_start_variant.as<float>();
      if (object.containsKey("temperature_end")) {
        JsonVariant temperature_end_variant = object["temperature_end"];
        if (!temperature_end_variant.is<float>()) {
          setError(errorMessage, "temperature_end must be a number");
          return false;
        }
        item.temperature_end = temperature_end_variant.as<float>();
      } else {
        item.temperature_end = item.temperature_start;
      }
    } else {
      item.temperature_start = 0.0f;
      item.temperature_end = 0.0f;
    }

    item.humidity_active = humidity_active_variant.as<bool>();
    if (item.humidity_active) {
      JsonVariant humidity_start_variant = object["humidity_start"];
      if (humidity_start_variant.isNull() || !humidity_start_variant.is<float>()) {
        setError(errorMessage, "humidity_start required when active");
        return false;
      }
      item.humidity_start = humidity_start_variant.as<float>();
      if (object.containsKey("humidity_end")) {
        JsonVariant humidity_end_variant = object["humidity_end"];
        if (!humidity_end_variant.is<float>()) {
          setError(errorMessage, "humidity_end must be a number");
          return false;
        }
        item.humidity_end = humidity_end_variant.as<float>();
      } else {
        item.humidity_end = item.humidity_start;
      }
    } else {
      item.humidity_start = 0.0f;
      item.humidity_end = 0.0f;
    }

    return true;
  }

  void onTick() {
    if (!active) {
      return;
    }

    offset += 1;
    if (offset >= items[pos].duration_seconds) {
      pos += 1;
      offset = 0;

      if (pos >= size) {
        debugf("Schedule completed");
        stop(false);
        return;
      }

      debugf("Schedule transitioned to step %d", pos + 1);
    }

    applyCurrentStep();
  }

  void applyCurrentStep() {
    if (!active || pos >= size) {
      return;
    }

    const ScheduleItem &item = items[pos];
    const float progress =
        item.duration_seconds <= 1
            ? 1.0f
            : ((float)offset / (float)(item.duration_seconds - 1));
    const float bounded_progress =
        progress < 0.0f ? 0.0f : (progress > 1.0f ? 1.0f : progress);

    ControlStatePublic nextState = getControlState();
    nextState.temperature_active = item.temperature_active;
    if (item.temperature_active) {
      nextState.target_temperature =
          item.temperature_start +
          (item.temperature_end - item.temperature_start) * bounded_progress;
    }
    nextState.humidity_active = item.humidity_active;
    if (item.humidity_active) {
      nextState.target_humidity =
          item.humidity_start +
          (item.humidity_end - item.humidity_start) * bounded_progress;
    }

    setControlState(nextState);
  }

private:
  ScheduleItem items[MaxScheduleItems];
  int32_t size;
  int32_t pos;    // Currently active item
  int32_t offset; // Offset inside current item in seconds;
  bool active;
  bool has_snapshot;
  ControlStatePublic snapshot;
  Timer timer;
};

Schedule currentSchedule;

bool startSchedule(String &name, String *errorMessage) {
  String filename = getScheduleFileName(name);
  if (currentSchedule.isActive()) {
    debugf("Schedule restart requested");
  }
  currentSchedule.stop(false);
  if (!currentSchedule.load(filename, errorMessage)) {
    return false;
  }

  return currentSchedule.start(errorMessage);
}

void stopSchedule() { currentSchedule.stop(true); }