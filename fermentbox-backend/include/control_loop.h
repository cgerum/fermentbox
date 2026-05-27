#pragma once

struct ControlStatePublic {
  float target_temperature;
  bool temperature_active;
  float target_humidity;
  bool humidity_active;
};

void startControlLoop();

ControlStatePublic getControlState();
