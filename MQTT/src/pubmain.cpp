#include <iostream>
#include "sensor.h"
#include "publisher.h"

int main(int argc, char* argv[]){

    if(argc < 2) {
        std::cout << "You need do set id for second command line argument" << std::endl;
        return 1;
    }

    Sensor s;
    Publisher p("localhost", argv[1], "test/t1", 1883, 60, s);

    p.pubSensor(1);
    return 0;
}
