#include "control_loop.h"

#include "configuration.h"
#include "nn_controller.h"
#include "sensors.h"

#include <SmingCore.h>

// All outputs are low active
#define HEATER_PIN 14     // D5
#define COOLER_PIN 12     // D6
#define VENTILATOR_PIN 13 // D7
#define HUMIDIFIER_PIN 2

#define HYSTERESIS_ON 0.028f
#define HYSTERESIS_OFF 0.017f
#define COOLDOWN 10

struct ControlLoopState {
  bool temperature_active;
  float target_temperature;

  bool humidity_active;
  float target_humidity;

  int heater_cooldown;
  int cooler_cooldown;
  int humidifier_cooldown;
  int ventilator_cooldown;

  bool heater_on;
  bool cooler_on;
  bool ventilator_on;
  bool humidifier_on;

  Timer timer;
};

static ControlLoopState state;

static bool fakeModeEnabled() { return FermentboxConfig::get().FakeMode; }

static bool &outputStateRef(uint8_t pin) {
  switch (pin) {
  case HEATER_PIN:
    return state.heater_on;
  case COOLER_PIN:
    return state.cooler_on;
  case VENTILATOR_PIN:
    return state.ventilator_on;
  case HUMIDIFIER_PIN:
    return state.humidifier_on;
  default:
    return state.heater_on;
  }
}

static bool isOutputActive(uint8_t pin) {
  if (fakeModeEnabled()) {
    return outputStateRef(pin);
  }
  return digitalRead(pin) == 0;
}

static void setOutputActive(uint8_t pin, bool active) {
  if (fakeModeEnabled()) {
    outputStateRef(pin) = active;
    return;
  }
  digitalWrite(pin, active ? 0 : 1);
}

static void setTemperatureOutputs(bool heater_on, bool cooler_on) {
  const bool heater_was_on = isOutputActive(HEATER_PIN);
  const bool cooler_was_on = isOutputActive(COOLER_PIN);

  if (heater_was_on != heater_on) {
    setOutputActive(HEATER_PIN, heater_on);
    if (!heater_on) {
      state.cooler_cooldown = COOLDOWN;
    }
  }
  if (cooler_was_on != cooler_on) {
    setOutputActive(COOLER_PIN, cooler_on);
    if (!cooler_on) {
      state.heater_cooldown = COOLDOWN;
    }
  }
}

static void applyThresholdTemperatureControl(
    const Sensors::Measurement &measurement) {
  bool heater_on = isOutputActive(HEATER_PIN);
  bool cooler_on = isOutputActive(COOLER_PIN);

  if (!state.temperature_active || measurement.error) {
    setTemperatureOutputs(false, false);
    return;
  }

  if (measurement.temperature > state.target_temperature * (1.0f - HYSTERESIS_OFF)) {
    heater_on = false;
  }
  if (measurement.temperature < state.target_temperature * (1.0f + HYSTERESIS_OFF)) {
    cooler_on = false;
  }
  if ((measurement.temperature > state.target_temperature * (1.0f + HYSTERESIS_ON)) &&
      state.cooler_cooldown <= 0) {
    cooler_on = true;
  }
  if ((measurement.temperature < state.target_temperature * (1.0f - HYSTERESIS_ON)) &&
      state.heater_cooldown <= 0) {
    heater_on = true;
  }

  setTemperatureOutputs(heater_on, cooler_on);
}

static void applyExperimentalTemperatureControl(
    const Sensors::Measurement &measurement) {
  TemperatureControllerInput input;
  input.temperature_active = state.temperature_active;
  input.target_temperature = state.target_temperature;
  input.current_temperature = measurement.temperature;
  input.measurement_error = measurement.error;
  input.heater_on = isOutputActive(HEATER_PIN);
  input.cooler_on = isOutputActive(COOLER_PIN);
  input.heater_cooldown = state.heater_cooldown;
  input.cooler_cooldown = state.cooler_cooldown;

  TemperatureControllerOutput output = {false, false};
  if (runExperimentalTemperatureController(input, output)) {
    setTemperatureOutputs(output.heater_on, output.cooler_on);
    return;
  }

  applyThresholdTemperatureControl(measurement);
}

void onControlStep() {

  Sensors &sensors = getSensors();
  Sensors::Measurement &measurement = sensors.getLastMeasurement();

  if (experimentalTemperatureControllerEnabled()) {
    applyExperimentalTemperatureControl(measurement);
  } else {
    applyThresholdTemperatureControl(measurement);
  }

  if (!state.humidity_active || measurement.error) {
    setOutputActive(VENTILATOR_PIN, false);
    setOutputActive(HUMIDIFIER_PIN, false);
  } else {

    if (measurement.humidity < state.target_humidity * (1.0 - HYSTERESIS_OFF)) {
      if (isOutputActive(VENTILATOR_PIN)) {
        setOutputActive(VENTILATOR_PIN, false);
        state.humidifier_cooldown = COOLDOWN;
      }
    }
    if (measurement.humidity > state.target_humidity * (1.0 + HYSTERESIS_OFF)) {
      if (isOutputActive(HUMIDIFIER_PIN)) {
        setOutputActive(HUMIDIFIER_PIN, false);
        state.ventilator_cooldown = COOLDOWN;
      }
    }
    if ((measurement.humidity >
         state.target_humidity * (1.0f + HYSTERESIS_ON)) &&
        state.ventilator_cooldown <= 0) {
      if (!isOutputActive(VENTILATOR_PIN)) {
        setOutputActive(VENTILATOR_PIN, true);
      }
    }
    if ((measurement.humidity <
         state.target_humidity * (1.0f - HYSTERESIS_ON)) &&
        state.humidifier_cooldown <= 0) {
      if (!isOutputActive(HUMIDIFIER_PIN)) {
        setOutputActive(HUMIDIFIER_PIN, true);
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
  state.humidity_active = false;
  state.target_temperature = 28.0f;
  state.target_humidity = 50.0f;
  state.heater_cooldown = 0;
  state.cooler_cooldown = 0;
  state.humidifier_cooldown = 0;
  state.ventilator_cooldown = 0;
  state.heater_on = false;
  state.cooler_on = false;
  state.ventilator_on = false;
  state.humidifier_on = false;
  resetExperimentalTemperatureController();

  // Initialize IO
  pinMode(COOLER_PIN, OUTPUT);
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(VENTILATOR_PIN, OUTPUT);
  pinMode(HUMIDIFIER_PIN, OUTPUT);

  setOutputActive(COOLER_PIN, false);
  setOutputActive(HEATER_PIN, false);
  setOutputActive(VENTILATOR_PIN, false);
  setOutputActive(HUMIDIFIER_PIN, false);
}

ControlStatePublic getControlState() {

  ControlStatePublic res;
  res.target_temperature = state.target_temperature;
  res.temperature_active = state.temperature_active;
  res.target_humidity = state.target_humidity;
  res.humidity_active = state.humidity_active;

  return res;
}

void setControlState(const ControlStatePublic &nextState) {
  state.target_temperature = nextState.target_temperature;
  state.temperature_active = nextState.temperature_active;
  state.target_humidity = nextState.target_humidity;
  state.humidity_active = nextState.humidity_active;
}

ActorStatePublic getActorState() {
  ActorStatePublic res;
  res.heater_on = isOutputActive(HEATER_PIN);
  res.cooler_on = isOutputActive(COOLER_PIN);
  res.ventilator_on = isOutputActive(VENTILATOR_PIN);
  res.humidifier_on = isOutputActive(HUMIDIFIER_PIN);
  return res;
}
