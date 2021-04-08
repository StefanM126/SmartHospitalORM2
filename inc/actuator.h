#ifndef ACTUATOR_H_INCLUDED
#define ACTUATOR_H_INCLUDED

#include<iostream>

class Actuator{
    char value_m;

    char charPtrToChar(char *value);

public:
    Actuator();
    Actuator(Actuator& a);
    void writeActuator(char *value);
};

#endif
