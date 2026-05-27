#pragma once

#include <SmingCore.h>

inline String getScheduleFileName(String &name) {
  String filename = String(".sch-") + name + ".json";
  return filename;
}

void startSchedule(String &name);

void stopSchedule();