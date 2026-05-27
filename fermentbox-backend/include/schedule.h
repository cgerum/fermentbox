#pragma once

#include <SmingCore.h>

inline String getScheduleFileName(String &name) {
  String filename = String(".sch-") + name + ".json";
  return filename;
}

bool startSchedule(String &name, String *errorMessage = nullptr);

void stopSchedule();