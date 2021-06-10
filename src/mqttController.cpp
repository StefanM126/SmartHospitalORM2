#include <mosquitto.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <pthread.h>
#include "ssdpClient.h"

#define MESSAGE_LEN 20
#define SLASH_NUM 2
#define BUFF_LEN  256

const char*  topics[] = {
    "bolnica/+/temperatura",
    "bolnica/+/spoljasnjeOsvetljenje",
    "bolnica/+/unutrasnjeOsvetljenje",
    "bolnica/+/senzorNaVratima"

    };


typedef struct {
    int temperatureThreshold;
    int indorLightThreshold;
    int outLightThreshold;

} room;

static room rooms[3];
enum SENSORS { TEMPERATURE, INDOR_LIGHT, DOOR, OUT_LIGHT };

void parseTopic(const char* inTopic, SENSORS* sensor, char* outTopic, int* room) {
        int i = 0;
        int numOfSlashes = 0;
        while (numOfSlashes < SLASH_NUM) {
            if (inTopic[i] == '/')
                numOfSlashes ++;
            i++;
        }

        *room = inTopic[i-2] - '0';
        char sen[BUFF_LEN] = {0};
        strcpy(sen, inTopic+i);
        strcpy(outTopic, inTopic);
        outTopic[i] = '\0';

        if (strcmp(sen, "temperatura") == 0) {
            strcat(outTopic, "upravljajTemperaturom");
            *sensor = TEMPERATURE;
        }
        else if (strcmp(sen, "spoljasnjeOsvetljenje") == 0) {
            strcat(outTopic, "upravljajRoletnama");
            *sensor = OUT_LIGHT;
        }
        else if (strcmp(sen, "unutrasnjeOsvetljenje") == 0) {
            strcat(outTopic, "upravljajOsvetljenjem");
            *sensor = INDOR_LIGHT;
        }
        else if (strcmp(sen, "senzorNaVratima") == 0) {
            strcat(outTopic, "upravljajZakljucavanjem");
            *sensor = DOOR;
        }
    }

    void answerMessage(SENSORS sensor, char* inMessage, char* outMessage, int room){

        switch (sensor) {
            case TEMPERATURE:
                if(*inMessage < rooms[room-1].temperatureThreshold){
                    sprintf(outMessage, "ukljuci");
                }
                else{
                    sprintf(outMessage, "iskljuci");
                }

                break;
            case INDOR_LIGHT:
                if(*inMessage < rooms[room-1].indorLightThreshold){
                    sprintf(outMessage, "ukljuci");
                }
                else{
                    sprintf(outMessage, "iskljuci");
                }
                break;
            case DOOR:
                sprintf(outMessage, "zakljucaj");
                break;
            case OUT_LIGHT:
                if(*inMessage > rooms[room-1].outLightThreshold){
                    sprintf(outMessage, "spusti");
                }
                else{
                    sprintf(outMessage, "podigni");
                }
                break;




        }

    }
    void on_connect(struct mosquitto *mosq, void *obj, int rc){
    	if (rc == 0) {

    		for(int i = 0; i < 4; i ++){

    		mosquitto_subscribe(mosq, NULL, topics[i], 0);
    		// printf("Subcribing to topic -t %s \n", topics[i]);


    		}

    	} else {
    		mosquitto_disconnect(mosq);
    	}
    }

    void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg){
    	// Print the topic for the first message received, then disconnect
    	// printf("Topic: %s Message: %d\n", msg->topic, *(char* )msg->payload);
        char outMessage[MESSAGE_LEN];
        SENSORS sensor;
        char outTopic[BUFF_LEN];
        int room;
        parseTopic(msg->topic, &sensor, outTopic, &room);
        answerMessage(sensor,(char* )msg->payload,  outMessage, room);
        mosquitto_publish(mosq, NULL, outTopic, MESSAGE_LEN , outMessage, 2, false);

    	// mosquitto_disconnect(mosq);
    }
    void* menuThreadFunction(void* params){
        while(1){
            unsigned int roomNum;
            unsigned int thresholdNum;
            printf("\e[1;1H\e[2J");
            printf("\033[%d;%dH", 0, 0);

            do{
                printf("Izaberite sobu\n" );
                printf(">>" );
                fflush(stdin);

                scanf("%d", &roomNum);
            }while(roomNum < 1 && roomNum > 3);
            do{
                printf("Izaberite senzor\n" );
                printf("1.Temperatura\n" );
                printf("2.Unutrasnje svetlo\n" );
                printf("3.Spoljasnje svetlo\n" );
                printf(">>" );
                fflush(stdin);

                scanf("%d", &thresholdNum);
                fflush(stdin);

            }while(thresholdNum < 1 && thresholdNum > 3);


            char choice = 0;
            while (choice != 'x') {
                printf("\e[1;1H\e[2J");
                printf("\033[%d;%dH", 0, 0);
                printf("Soba %d:\n", roomNum);
                switch (thresholdNum) {
                    case 1:
                        printf("\tTemperatura: %d\n", rooms[roomNum-1].temperatureThreshold );

                        break;
                    case 2:
                        printf("\tUnutrasnje svetlo: %d%%\n", rooms[roomNum-1].indorLightThreshold );

                        break;
                    case 3:
                        printf("\tSpoljasnje svetlo: %d%%\n", rooms[roomNum-1].outLightThreshold );
                        break;
                        }

                printf("\t1. Pojacaj -> +\n" );
                printf("\t2. Smanji  -> -\n" );
                printf("\t3. Izlaz   -> x\n" );
                printf("\t>>" );
                fflush(stdin);
                getc(stdin);
                choice = getchar();
                switch (thresholdNum) {
                    case 1:

                        if(choice == '+'){
                            rooms[roomNum-1].temperatureThreshold++;
                        }else if(choice == '-'){
                            rooms[roomNum-1].temperatureThreshold--;
                        }

                        break;
                    case 2:
                        if(choice == '+'){
                            rooms[roomNum-1].indorLightThreshold++;
                        }else if(choice == '-'){
                            rooms[roomNum-1].indorLightThreshold--;
                        }
                        break;
                    case 3:
                        if(choice == '+'){
                            rooms[roomNum-1].outLightThreshold++;
                        }else if(choice == '-'){

                            rooms[roomNum-1].outLightThreshold--;
                        }
                        break;
                        }
            }
        }
        return NULL;
    }
    
int main()
{
	struct mosquitto *mosq;
	int rc;

	mosquitto_lib_init();


	mosq = mosquitto_new(NULL, true, NULL);
	if (mosq == NULL) {
		printf("Failed to create client instance.\n");
		return 1;
	}
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);

    initSSDPClient("Controller", "controller_controller.json");
    pthread_t StartSSDPClient;
    pthread_create( & StartSSDPClient, NULL, StartSSDPClientFunction, NULL);

    std::string addres = getBrokerAddres();
    std::cout << addres << std::endl;

	rc = mosquitto_connect(mosq, addres.c_str(), 1883, 60);
	if (rc != MOSQ_ERR_SUCCESS) {
		printf("Connect failed: %s\n", mosquitto_strerror(rc));
		return 1;
	}
    room room1, room2, room3;
    room1.temperatureThreshold = 22;
    room1.indorLightThreshold = 50;
    room1.outLightThreshold = 70;
    room2.temperatureThreshold = 22;
    room2.indorLightThreshold = 50;
    room2.outLightThreshold = 70;
    room3.temperatureThreshold = 22;
    room3.indorLightThreshold = 50;
    room3.outLightThreshold = 70;
    rooms[0] = room1;
    rooms[1] = room2;
    rooms[2] = room3;
    pthread_t menuThread;

    pthread_create(&menuThread, NULL, menuThreadFunction, NULL);


	mosquitto_loop_forever(mosq, -1, 1);
    pthread_join(StartSSDPClient, NULL);
	mosquitto_destroy(mosq);

	mosquitto_lib_cleanup();
}
