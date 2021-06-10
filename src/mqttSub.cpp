// Subscriber
#include <mosquitto.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include "ssdpClient.h"

const char*  topic;

static char* actuator_state;
void set_actuator(char* state) {
	actuator_state = state;
}

void on_connect(struct mosquitto *mosq, void *obj, int rc)
{
	if (rc == 0) {


		mosquitto_subscribe(mosq, NULL, topic, 0);
		printf("Subcribing to topic -t %s \n", topic);



	} else {
		mosquitto_disconnect(mosq);
	}
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg)
{
	// Print the topic for the first message received, then disconnect
	printf("Topic: %s Message: %s\n", msg->topic, (char* )msg->payload);
	set_actuator(msg->payload);
}

int main(int argc, char* argv[]) {

	struct mosquitto *mosq;
	int rc;

	mosquitto_lib_init();

	mosq = mosquitto_new(NULL, true, NULL);
	if (mosq == NULL) {
		printf("Failed to create client instance.\n");
		return 1;
	}

    if (argc < 4) {
        std::cout << "To few arguments (need 4); Input format: ./pub <client_name> <client_attribute> <topic>" << std::endl;
        exit(1);
    }

	topic = argv[3];
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);

    initSSDPClient(argv[1], argv[2]);
    pthread_t StartSSDPClient;
    pthread_create( & StartSSDPClient, NULL, StartSSDPClientFunction, NULL);

    std::string addres = getBrokerAddres();
    std::cout << addres << std::endl;

	rc = mosquitto_connect(mosq, addres.c_str(), 1883, 60);
	if (rc != MOSQ_ERR_SUCCESS) {
		printf("Connect failed: %s\n", mosquitto_strerror(rc));
		return 1;
	}

	mosquitto_loop_forever(mosq, -1, 1);
    pthread_join(StartSSDPClient, NULL);

	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
}
