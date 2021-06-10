// Publisher
#include <stdio.h>
#include <mosquitto.h>
#include <string>
#include <unistd.h>
#include "ssdpClient.h"


int main(int argc, char* argv[]) {
	int rc;
	struct mosquitto * mosq;

	mosquitto_lib_init();

	mosq = mosquitto_new("publisher-test", true, NULL);

    if (argc < 4) {
        std::cout << "To few arguments (need 4); Input format: ./pub <client_name> <client_attribute> <topic>" << std::endl;
        exit(1);
    }

    initSSDPClient(argv[1], argv[2]);
    pthread_t StartSSDPClient;
    pthread_create( & StartSSDPClient, NULL, StartSSDPClientFunction, NULL);

    std::string addres = getBrokerAddres();
    std::cout << addres << std::endl;

	rc = mosquitto_connect(mosq, addres.c_str(), 1883, 60);
	if (rc != 0) {
		printf("Client could not connect to broker! Error Code: %d\n", rc);
		mosquitto_destroy(mosq);
		return -1;
	}

	const char*  topic = argv[3];

	printf("We are now connected to the broker!\n");
	char i = 0;

	while(1){
		mosquitto_publish(mosq, NULL, topic, 4, (void*)&i, 2, false);

		mosquitto_loop_start(mosq);
		sleep(10);
		i += 20;

		mosquitto_loop_stop(mosq, true);
	}
    pthread_join(StartSSDPClient, NULL);

	return 0;
}
