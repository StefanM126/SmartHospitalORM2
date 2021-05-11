#include "jsonParser.h"

#define JSON_PATH std::string("../json/")

int main() {
    ControllerClass con(JSON_PATH + "cc.json");
    ClientClass cl(JSON_PATH + "c.json");

    std::cout << "-------------------------" << std::endl;
    std::cout << "Primer klijentskog fajla:" << std::endl;
    std::cout << cl << std::endl;

    std::cout << "-------------------------" << std::endl;
    std::cout << "Primer kotroler fajla:" << std::endl;
    std::cout << con << std::endl;

    return 0;
}

// g++ -I../inc/ json.cpp ../src/jsonParser.cpp ../src/jsoncpp.cpp
