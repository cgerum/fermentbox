#pragma once

struct ControlStatePublic {
  float target_temperature;
  bool temperature_active;
  float target_humidity;
  bool humidity_active;
};

struct ActorStatePublic {
  bool heater_on;
  bool cooler_on;
  bool ventilator_on;
  bool humidifier_on;
};

void startControlLoop();

ControlStatePublic getControlState();
ActorStatePublic getActorState();

void setControlState(const ControlStatePublic &nextState);
