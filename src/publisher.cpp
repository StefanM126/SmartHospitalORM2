#include <iostream>
#include <mosquittopp.h>
#include <chrono>
#include <thread>
#include <cstring>
#include <mutex>
#include "sensor.h"
#include "publisher.h"

void Publisher::on_connect(int rc) {
    std::cout << "Connected with id: " << id_m << std::endl;
    if(rc) {
        std::cout << "Error with result code: " << rc << std::endl;
        exit(1);
    } 
}
void Publisher::on_disconnect(int rc) {
    std::cout << "Disconnected" << std::endl;
    if(rc) {
        std::cout << "Error with result code: " << rc << std::endl;
        exit(1);
    } 
}
void Publisher::on_publish(int mid) {
    std::unique_lock<std::mutex> lock(mx);
    std::cout << "Publisher with id: "<< id_m << " published on topic: " << topic_m <<  " mid = " << mid << std::endl;
}

bool Publisher::pubSensor(int sec){
    int ret;
    while((ret = publish(NULL, topic_m, strlen(sensor_m.readSensor()), sensor_m.readSensor()) == 0)) {
        std::unique_lock<std::mutex> lock(mx);
        std::this_thread::sleep_for(std::chrono::seconds(sec));            
    }

    if(ret)
        return false;
    return true;
}

Publisher::Publisher(const char* host, const char* id, const char* topic, int port, int keepalive, Sensor &sensor) : sensor_m(sensor) {
    mosqpp::lib_init();

    host_m = host;
    id_m = id;
    topic_m = topic;
    port_m = port;
    keepalive_m = keepalive;

    connect_async(host, port, keepalive);
    loop_start();
}


Publisher::~Publisher() {
    loop_stop();
    mosqpp::lib_cleanup();
}






    

