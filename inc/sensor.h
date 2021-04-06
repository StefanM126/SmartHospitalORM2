#ifndef SENSOR_H_INCLUDED
#define SENSOR_H_INCLUDED

#include<iostream>
#include<cstdlib>

class Sensor{
    char value_m;

    char* charToCharPtr(char value);
public:
    Sensor();
    char* readSensor();
};

#endif
