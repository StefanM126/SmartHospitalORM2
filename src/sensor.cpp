#include "sensor.h"

Sensor::Sensor() : value_m(0) {}

char* Sensor::charToCharPtr(char value) {
    static char ret[3] = {0};
    int i = 3;
    for(; i > -1 && value; i--, value /= 10) {
       ret[i] = value % 10 + '0';
    }
    return &ret[i+1];
}

char* Sensor::readSensor() {
    return charToCharPtr(value_m = rand() % 101);
}
