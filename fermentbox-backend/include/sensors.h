#pragma once 

#include <SmingCore.h>

class Sensors {
    public:
    struct Measurement {
        DateTime date;
        float temperature;
        float humidity;
    };

    void start();
    Measurement &getLastMeasurement() { return lastMeasurement; }

    void onTimer();

    protected:

    void readMeasurement(Measurement &ms);

    private:
    Measurement lastMeasurement;
    Timer timer;
};

Sensors& getSensors();
void startSensors();