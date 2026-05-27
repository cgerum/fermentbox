#include <cstdlib>

#include <Libraries/DHTesp/DHTesp.h>
#include <SmingCore.h>

#include "configuration.h"
#include "control_loop.h"
#include "sensors.h"
#include "status.h"

#define DHT_WORK_PIN 16

static DHTesp dht;
static Sensors currentSensors;
static bool simulationInitialized = false;
static float simulatedTemperature = 20.0f;
static float simulatedHumidity = 50.0f;

static float clamp(float value, float min, float max) {
  if (value < min) {
    return min;
  }
  if (value > max) {
    return max;
  }
  return value;
}

static void readSimulatedMeasurement(Sensors::Measurement &ms) {
  const ActorStatePublic actors = getActorState();
  const float noise = static_cast<float>((rand() % 200) - 100) / 1000.0f;
  const float humidityNoise = static_cast<float>((rand() % 200) - 100) / 500.0f;

  if (!simulationInitialized) {
    simulationInitialized = true;
    ControlStatePublic control = getControlState();
    simulatedTemperature = control.target_temperature;
    simulatedHumidity = control.target_humidity;
  }

  simulatedTemperature += (20.0f - simulatedTemperature) * 0.02f;
  simulatedTemperature += actors.heater_on ? 0.08f : 0.0f;
  simulatedTemperature -= actors.cooler_on ? 0.08f : 0.0f;
  simulatedTemperature += noise;
  simulatedTemperature = clamp(simulatedTemperature, -20.0f, 80.0f);

  simulatedHumidity += (50.0f - simulatedHumidity) * 0.02f;
  simulatedHumidity += actors.humidifier_on ? 0.35f : 0.0f;
  simulatedHumidity -= actors.ventilator_on ? 0.35f : 0.0f;
  simulatedHumidity += humidityNoise;
  simulatedHumidity = clamp(simulatedHumidity, 0.0f, 100.0f);

  ms.date = SystemClock.now();
  ms.temperature = simulatedTemperature;
  ms.humidity = simulatedHumidity;
  ms.error = false;
  clear_status(STATUS_SENSOR_FAILED);
}

Sensors &getSensors() { return currentSensors; }

void startSensors() {
  if (!FermentboxConfig::get().FakeMode) {
    dht.setup(DHT_WORK_PIN, DHTesp::DHT22);
  }
  currentSensors.start();
}

void Sensors::readMeasurement(Measurement &ms) {
  if (FermentboxConfig::get().FakeMode) {
    readSimulatedMeasurement(ms);
    return;
  }

  DateTime currentDate = SystemClock.now();
  TempAndHumidity th = dht.getTempAndHumidity();

  if (dht.getStatus() == DHTesp::ERROR_NONE) {
    ms.date = currentDate;
    ms.temperature = th.temperature;
    ms.humidity = th.humidity;
    ms.error = false;
    clear_status(STATUS_SENSOR_FAILED);
  } else {
    Serial.print("Failed to read from DHT: ");
    Serial.print(dht.getStatus());
    Serial.print("\n");
    add_status(STATUS_SENSOR_FAILED);
    ms.error = true;
  }
}

static void timerCallback() { currentSensors.onTimer(); }

void Sensors::start() {
  const uint32_t samplingPeriodMs =
      FermentboxConfig::get().FakeMode ? 1000 : dht.getMinimumSamplingPeriod();
  timer.initializeMs(samplingPeriodMs, timerCallback).start();
}

void Sensors::onTimer() {
  readMeasurement(lastMeasurement);
  // debugf("New measurement temp: %f, hum: %f\n", lastMeasurement.temperature,
  //       lastMeasurement.humidity);
}
