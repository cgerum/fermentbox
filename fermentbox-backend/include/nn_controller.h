#pragma once

struct TemperatureControllerInput {
  bool temperature_active;
  float target_temperature;
  float current_temperature;
  bool measurement_error;
  bool heater_on;
  bool cooler_on;
  int heater_cooldown;
  int cooler_cooldown;
};

struct TemperatureControllerOutput {
  bool heater_on;
  bool cooler_on;
};

bool experimentalTemperatureControllerEnabled();

void resetExperimentalTemperatureController();

bool runExperimentalTemperatureController(const TemperatureControllerInput &input,
                                          TemperatureControllerOutput &output);
