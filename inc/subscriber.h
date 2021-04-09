#ifndef SUBSCRIBER_H_INCLUDED
#define SUBSCRIBER_H_INCLUDED

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <mutex>
#include <mosquittopp.h>
#include "actuator.h"


class Subscriber : public mosqpp::mosquittopp {
    const char* host_m;
    const char* id_m;
    const char* topic_m;
    int port_m;
    int keepalive_m;
    Actuator actuator_m;
    std::mutex mx;

    void on_connect(int rc);
    void on_disconnect(int rc);
    void on_message(const struct mosquitto_message *msg);

public:
    Subscriber(const char* host, const char* id, const char* topic, int port, int keepalive, Actuator& actuator);
    ~Subscriber();
};

#endif
