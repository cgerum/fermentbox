#include <cstdlib>

#include <SmingCore.h>
#include <Libraries/DHTesp/DHTesp.h>

#include "sensors.h"


#define WORK_PIN 16

static DHTesp dht;
static Sensors currentSensors; 

Sensors& getSensors(){
    return currentSensors;
}

void startSensors() {
    currentSensors.start();
    dht.setup(WORK_PIN, DHTesp::DHT22);
}

void Sensors::readMeasurement(Measurement &ms){
    int random = rand();

    
	DateTime currentDate = SystemClock.now();
    TempAndHumidity th = dht.getTempAndHumidity();

    if (dht.getStatus() == DHTesp::ERROR_NONE){
        ms.date = currentDate;
        ms.temperature = th.temperature;
        ms.humidity = th.humidity;
    }else{
        Serial.print("Failed to read from DHT: ");
        Serial.print(dht.getStatus());
        Serial.print("\n");
    }
}


static void timerCallback(){
    currentSensors.onTimer();
}

void Sensors::start(){
    timer.initializeMs(2*1000, timerCallback).start();
}

void Sensors::onTimer(){
    readMeasurement(lastMeasurement);
    debugf("New measurement temp: %f, hum: %f\n", lastMeasurement.temperature, lastMeasurement.humidity);
}
