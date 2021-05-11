#include "messageParser.h"

std::string Message::parseOne(std::string& msg) {
    std::string ret = "";
    int found;

    if((found = msg.find(":")) == (int)std::string::npos) {
        ret = msg.substr(0, msg.size());
        msg.clear();
    }
    else {
        ret = msg.substr(0, found);
        msg.erase(0, found + 1);
    }
    return ret;
}

std::string Message::getReceiverId() {
    return receiverId;
}

std::string Message::getSenderId() {
    return senderId;
}

void Message::setReceiverId(std::string receiverId) {
    this->receiverId = receiverId;
}

void Message::setSenderId(std::string senderId) {
    this->senderId = senderId;
}

bool ClientMessage::parse(std::string msg) {
    receiverId = parseOne(msg);
    senderId = parseOne(msg);
    
    if (msg == "") 
        return false;

    type = parseOne(msg);
    purpose = parseOne(msg);
    return true;
}

std::string ClientMessage::getType() {
    return type;
}

std::string ClientMessage::getPurpose() {
    return purpose;
}

void ClientMessage::setType(std::string type) {
    this->type = type;
}

void ClientMessage::setPuropse(std::string purpose) {
    this->purpose = purpose;
}

bool ControllerMessage::parse(std::string msg) {
        receiverId         = parseOne(msg);
        senderId           = parseOne(msg);
        std::string stat   = parseOne(msg); 

    if(stat.compare("OK") == 0) {
        state = OK;
        brokerIP           = parseOne(msg);

        std::string keepAl = parseOne(msg); 
        std::stringstream str(keepAl);
        str >> keepAlive;
    }
    else if(stat.compare("RLSD") == 0) {
        state = RLSD;
        brokerIP = "";
        keepAlive = 0;
    }
    else {
        state = NOK;
        brokerIP = "";
        keepAlive = 0;
    }
    return true;
}

ControllerMsgState ControllerMessage::getState() {
    return state; 
}


std::string ControllerMessage::getBrokerIP() {
    return brokerIP;
}

int ControllerMessage::getKeepAlive() {
    return keepAlive; 
}

void ControllerMessage::setBrokerIP(std::string brokerIP) {
    this->brokerIP = brokerIP;
}

void ControllerMessage::setState(ControllerMsgState state) {
    this->state = state;
}

void ControllerMessage::setKeepAlive(int keepAlive) {
    this->keepAlive = keepAlive;
}

