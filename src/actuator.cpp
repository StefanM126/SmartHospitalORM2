#include "actuator.h"
#define INIT_VALUE 0

Actuator::Actuator() : value_m(INIT_VALUE) {}

void Actuator::writeActuator(char *value) {
    value_m = charPtrToChar(value); 
}

char Actuator::charPtrToChar(char* value) {
    char ret = 0;
    int i = 0;

    while (value[i] != '\0') {
        ret *= 10;
        ret += value[i] - '0';
    }
    return ret;
}
