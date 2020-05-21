#include "control_loop.h"

#include "sensors.h"

#include <SmingCore.h>

// All outputs are low active
#define HEATER_PIN 14     // D5
#define COOLER_PIN 12     // D6
#define VENTILATOR_PIN 13 // D7
#define HUMIDIFIER_PIN 2

#define HYSTERESIS 0.03f
#define COOLDOWN 180

struct ControlLoopState {
  bool temperature_active;
  float target_temperature;

  bool humidity_active;
  float target_humidity;

  int heater_cooldown;
  int cooler_cooldown;
  int humidifier_cooldown;
  int ventilator_cooldown;

  Timer timer;
};

static ControlLoopState state;

void onControlStep() {

  Sensors &sensors = getSensors();
  Sensors::Measurement &measurement = sensors.getLastMeasurement();

  if (!state.temperature_active) {
    digitalWrite(HEATER_PIN, 1);
    digitalWrite(COOLER_PIN, 1);
  } else {
    if (measurement.temperature > state.target_temperature) {
      if (digitalRead(HEATER_PIN) == 0) {
        digitalWrite(HEATER_PIN, 1);
        state.cooler_cooldown = COOLDOWN;
      }
    }
    if (measurement.temperature < state.target_temperature) {
      if (digitalRead(COOLER_PIN) == 0) {
        digitalWrite(COOLER_PIN, 1);
        state.heater_cooldown = COOLDOWN;
      }
    }
    if ((measurement.temperature >
         state.target_temperature * (1.0f + HYSTERESIS)) &&
        state.cooler_cooldown <= 0) {
      if (digitalRead(COOLER_PIN) == 1) {
        digitalWrite(COOLER_PIN, 0);
      }
    }

    if ((measurement.temperature <
         state.target_temperature * (1.0f - HYSTERESIS)) &&
        state.heater_cooldown <= 0) {
      if (digitalRead(HEATER_PIN) == 1) {
        digitalWrite(HEATER_PIN, 0);
      }
    }
  }

  if (!state.humidity_active) {
    digitalWrite(VENTILATOR_PIN, 1);
    digitalWrite(HUMIDIFIER_PIN, 1);
  } else {

    if (measurement.humidity < state.target_humidity) {
      if (digitalRead(VENTILATOR_PIN) == 0) {
        digitalWrite(VENTILATOR_PIN, 1);
        state.humidifier_cooldown = COOLDOWN;
      }
    }
    if (measurement.humidity > state.target_humidity) {
      if (digitalRead(HUMIDIFIER_PIN) == 0) {
        digitalWrite(HUMIDIFIER_PIN, 1);
        state.ventilator_cooldown = COOLDOWN;
      }
    }
    if ((measurement.humidity > state.target_humidity * (1.0f + HYSTERESIS)) &&
        state.ventilator_cooldown <= 0) {
      if (digitalRead(VENTILATOR_PIN) == 1) {
        digitalWrite(VENTILATOR_PIN, 0);
      }
    }
    if ((measurement.humidity < state.target_humidity * (1.0f - HYSTERESIS)) &&
        state.humidifier_cooldown <= 0) {
      if (digitalRead(HUMIDIFIER_PIN) == 1) {
        digitalWrite(HUMIDIFIER_PIN, 0);
      }
    }
  }

  if (state.heater_cooldown > 0) {
    state.heater_cooldown -= 1;
  }
  if (state.cooler_cooldown > 0) {
    state.cooler_cooldown -= 1;
  }
  if (state.ventilator_cooldown > 0) {
    state.ventilator_cooldown -= 1;
  }
  if (state.humidifier_cooldown > 0) {
    state.humidifier_cooldown -= 1;
  }

  if (state.heater_cooldown) {
    debugf("state.heater_cooldown: %d\n", state.heater_cooldown);
  }
  if (state.cooler_cooldown) {
    debugf("state.cooler_cooldown: %d\n", state.cooler_cooldown);
  }
  if (state.ventilator_cooldown) {
    debugf("state.ventilator_cooldown: %d\n", state.ventilator_cooldown);
  }
  if (state.humidifier_cooldown) {
    debugf("state.humidifier_cooldown: %d\n", state.humidifier_cooldown);
  }
}

void startControlLoop() {
  state.timer.initializeMs(1000, onControlStep).start();

  state.temperature_active = true;
  state.humidity_active = true;
  state.target_temperature = 30.0f;
  state.target_humidity = 50.0f;

  // Initialize IO
  pinMode(COOLER_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(VENTILATOR_PIN, OUTPUT);
  pinMode(HUMIDIFIER_PIN, OUTPUT);

  digitalWrite(COOLER_PIN, 1);
  digitalWrite(HEATER_PIN, 1);
  digitalWrite(VENTILATOR_PIN, 1);
  digitalWrite(HUMIDIFIER_PIN, 1);
}

ControlStatePublic getControlState() {

  ControlStatePublic res;
  res.target_temperature = state.target_temperature;
  res.temperature_active = state.temperature_active;
  res.target_humidity = state.target_humidity;
  res.humidity_active = state.humidity_active;

  return res;
}