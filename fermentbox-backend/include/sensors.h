#pragma once 

class Sensors {
    public:
    struct Measurement {
        float temperature;
        float humidity;
    };

    void readMeasurement(Measurement &ms);
    void start();
    void onTimer();
    
    private:
    Measurement lastMeasurement;
    Timer timer;
};

Sensors& getSensors();
void startSensors();