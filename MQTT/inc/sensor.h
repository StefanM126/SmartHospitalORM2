#ifndef SENSOR_H_INCLUDED
#define SENSOR_H_INCLUDED

#include<iostream>
#include<cstdlib>

class Sensor{
    unsigned char value_m;

    char* charToCharPtr(unsigned char value);
public:
    Sensor();
    Sensor(Sensor &s);
    char* readSensor();
};

#endif
