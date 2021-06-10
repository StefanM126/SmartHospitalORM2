#include <iostream>
#include "ssdpController.h"

int main(int argc, char * argv[]) {
    initSSDPController("controller.json");
    pthread_t StartSSDPController; 
    pthread_create( & StartSSDPController, NULL, StartSSDPControllerFunction, NULL);

    pthread_join(StartSSDPController, NULL);
    return 0;
}

