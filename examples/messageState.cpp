#include "messageParser.h"

int main() {

    std::string msg0 = "dfa:faf:NOK";
    std::string msg1 = "dfa:faf:RLSD";
    std::string msg2 = "dfa:faf:OK:fdfkdsa:10";

    ControllerMessage cm;
    cm.parse(msg0);
    std::cout << cm.getState() << std::endl;
    cm.parse(msg1);
    std::cout << cm.getState() << std::endl;
    cm.parse(msg2);
    std::cout << cm.getState() << std::endl;

    return 0;
}

/*
g++ -I../inc messageState.cpp ../src/messageParser.cpp
*/
