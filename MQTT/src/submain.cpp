#include <iostream>
#include "actuator.h"
#include "subscriber.h"


int main() {
    Actuator a;
    Subscriber s("localhost", "1", "test/t1", 1883, 60, a);

    std::cout << "Press Enter to quit..." << std::endl;
    getchar();

    return 0;
}
