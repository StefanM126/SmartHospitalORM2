#include "sensor.h"

Sensor::Sensor() : value_m(0) {}

Sensor::Sensor(Sensor &s) {
    value_m = s.value_m;
}

char* Sensor::charToCharPtr(unsigned char value) {
    static char ret[4] = {0};
    int i = 3;
    for(; i > -1 && value; i--, value /= 10) {
       ret[i] = value % 10 + '0';
    }
    return &ret[i+1];
}

char* Sensor::readSensor() {
    return charToCharPtr(value_m = rand() % 101);
}
