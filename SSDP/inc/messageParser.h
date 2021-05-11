#ifndef MESSAGE_PARSER_H_INCLUDED
#define MESSAGE_PARSER_H_INCLUDED

#include <iostream>
#include <string>
#include <cstdlib>
#include <sstream>

enum ControllerMsgState {OK, NOK, RLSD};

class Message {
protected:
    std::string receiverId; 
    std::string senderId; 

    std::string parseOne(std::string& msg); 

public:
    virtual bool parse(std::string msg) = 0;
    
    std::string getReceiverId();
    std::string getSenderId();

    void setReceiverId(std::string receiverId);
    void setSenderId(std::string senderId);
}; 

class ClientMessage : public Message {
    std::string type;
    std::string purpose;
 
public:
    bool parse(std::string msg);
   
    std::string getType();
    std::string getPurpose();

    void setType(std::string type);
    void setPuropse(std::string purpose);
};

class ControllerMessage : public Message {
    ControllerMsgState state;
    int keepAlive;
    std::string brokerIP;

public:
    bool parse(std::string msg);

    ControllerMsgState getState();
    std::string getBrokerIP();
    int getKeepAlive();

    void setBrokerIP(std::string brokerIP);
    void setState(ControllerMsgState state);
    void setKeepAlive(int keepAlive);
};

#endif
