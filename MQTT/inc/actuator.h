#ifndef ACTUATOR_H_INCLUDED
#define ACTUATOR_H_INCLUDED

#include<iostream>

class Actuator{
    unsigned char value_m;

    unsigned char charPtrToChar(char *value);

public:
    Actuator();
    Actuator(Actuator& a);
    void writeActuator(char *value);
};

#endif
