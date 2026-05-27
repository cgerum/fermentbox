#include <cstdlib>

#include <Libraries/DHTesp/DHTesp.h>
#include <SmingCore.h>

#include "sensors.h"
#include "status.h"

#define DHT_WORK_PIN 16

static DHTesp dht;
static Sensors currentSensors;

Sensors &getSensors() { return currentSensors; }

void startSensors() {
  dht.setup(DHT_WORK_PIN, DHTesp::DHT22);
  currentSensors.start();
}

void Sensors::readMeasurement(Measurement &ms) {
  int random = rand();

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
  timer.initializeMs(dht.getMinimumSamplingPeriod(), timerCallback).start();
}

void Sensors::onTimer() {
  readMeasurement(lastMeasurement);
  // debugf("New measurement temp: %f, hum: %f\n", lastMeasurement.temperature,
  //       lastMeasurement.humidity);
}
