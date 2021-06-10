#ifndef JSONPARSER_H_INCLUDED
#define JSONPARSER_H_INCLUDED

#include <iostream>
#include <string>
#include <fstream>
#include <list>

#include "json.h"

class JsonParser {
protected:
    std::string data;
    Json::Value value;

public:

    JsonParser();

    JsonParser(std::string filename);
};

class ClientClass : public JsonParser {
    std::string type;
    std::string attribute;

public:

    ClientClass();

    ClientClass(std::string filename);
    ClientClass(std::string type, std::string attribute);

    std::string getType();
    std::string getAttribute();

    bool operator==(const ClientClass& other);

    friend std::ostream& operator<<(std::ostream& os, const ClientClass& c);

};

class ControllerClass : public JsonParser {
    std::list<ClientClass> clients;

public:

	std::list<ClientClass> getClients();
    ControllerClass();
    ControllerClass(std::string filename);


    bool isInList(std::string type, std::string attribute);

    friend std::ostream& operator<<(std::ostream& os, const ControllerClass& c);
};

#endif
