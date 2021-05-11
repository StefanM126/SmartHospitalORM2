#include "jsonParser.h"

JsonParser::JsonParser() {}

JsonParser::JsonParser(std::string filename) {
    std::filebuf fb;

    if (fb.open(filename, std::ios::in)) {
        std::istream is(&fb);
        Json::CharReaderBuilder builder;
        builder["collectComments"] = false;
        std::string errs;

        bool ok = parseFromStream(builder, is, &value, &errs);

        if (!ok) {
            exit(1);
        }
            
    }
}
    
std::string ClientClass::getType() {
	return type;
}

std::string ClientClass::getAttribute() {
	return attribute;
}

ClientClass::ClientClass(std::string filename) : JsonParser(filename) {
    type = value["type"].asString();
    attribute = value["attribute"].asString();
} 

ClientClass::ClientClass(std::string type, std::string attribute) {
   this->type = type; 
   this->attribute = attribute; 
}

bool ClientClass::operator==(const ClientClass& other) { 
    return other.type == this->type && other.attribute == this->attribute;
}    

std::ostream& operator<<(std::ostream& os, const ClientClass& c) {
	os << "Type: " << c.type << std::endl;
	os << "Attribute: " << c.attribute;
	return os;
}


ControllerClass::ControllerClass(std::string filename) : JsonParser(filename) {
    for(unsigned i = 0; i < value["clients"].size(); i++) {
        ClientClass c(value["clients"][i]["type"].asString(), value["clients"][i]["attribute"].asString());
        clients.push_back(c);
    }
} 

std::list<ClientClass> ControllerClass::getClients() {
	return clients;
}

bool ControllerClass::isInList(std::string type, std::string attribute) {
    ClientClass tmp(type, attribute);
    for(auto it = clients.begin(); it != clients.end(); it++) {
        if( (*it) == tmp)
            return true;
    }
    return false;
}

std::ostream& operator<<(std::ostream& os, const ControllerClass& c) {
	std::list<ClientClass>::const_iterator it = c.clients.begin();

	for(unsigned i = 0; i < c.value["clients"].size(); i++) {
		 os << "---------- " + std::to_string(i+1) + ". ----------" << std::endl;
		 os << *it << std::endl;
		 it++;
	}
	return os;
}
