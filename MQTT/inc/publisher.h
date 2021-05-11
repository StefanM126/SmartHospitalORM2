#ifndef PUBLISHER_H_INCLUDED
#define PUBLISHER_H_INCLUDED

#include <iostream>
#include <mosquittopp.h>
#include <chrono>
#include <thread>
#include <cstring>
#include <mutex>
#include "sensor.h"

class Publisher : public mosqpp::mosquittopp {
    const char* host_m;
    const char* id_m;
    const char* topic_m;
    int port_m;
    int keepalive_m;
    Sensor sensor_m;
    std::mutex mx;

    void on_connect(int rc);
    void on_disconnect(int rc);
    void on_publish(int mid);

public:
    bool pubSensor(int sec);
    Publisher(const char* host, const char* id, const char* topic, int port, int keepalive, Sensor &sensor);
    ~Publisher();
};

#endif
