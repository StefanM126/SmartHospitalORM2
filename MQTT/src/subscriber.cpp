#include "subscriber.h"

void Subscriber::on_connect(int rc) {
    if(rc) {
        std::cout << "Error with result code: " << rc << std::endl;
        exit(1);
    }

    if((rc = subscribe(NULL, topic_m)) != 0) {
        std::cout << "Error with result code: " << rc << std::endl;
        exit(1);
    }

    std::cout << "Connected" << std::endl;
}

void Subscriber::on_message(const struct mosquitto_message *msg) {
    std::unique_lock<std::mutex> lock(mx);
    std::cout << "New message with topic " << msg->topic << ": " << (char*) msg->payload << std::endl;
    actuator_m.writeActuator((char*) msg->payload);
}

void Subscriber::on_disconnect(int rc) {
    if(rc) {
        std::cout << "Error with result code: " << rc << std::endl;
        exit(1);
    }
    
    std::cout << "Disconnected" << std::endl;
}

Subscriber::Subscriber(const char* host, const char* id, const char* topic, int port, int keepalive, Actuator &actuator) : actuator_m(actuator) {

    mosqpp::lib_init();

    host_m = host;
    id_m = id;
    topic_m = topic;
    port_m = port;
    keepalive_m = keepalive;

    connect_async(host, port, keepalive);
    loop_start();
}

Subscriber::~Subscriber() {
    loop_stop();
    mosqpp::lib_cleanup();
}

