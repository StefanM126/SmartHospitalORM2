#ifndef CONTROLLER_H_INCLUED
#define CONTROLLER_H_INCLUED

//----- Include files -------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <string>
#include <list>
#include <mutex>
#include "messageParser.h"
#include "jsonParser.h"

//----- Defines -------------------------------------------------------------
#define PORT_NUM 1900
#define GROUP_ADDR "239.255.255.250"
#define BUFF_LEN 256

#define CONTROLLER_NAME "Controller_1"
#define BROKER_IP "192.168.1.1"

#define JSON_PATH std::string("../json/")

#define TAG_ALIVE       "ALIVE:\t"
#define TAG_OK          "OK:\t"
#define TAG_NOK         "NOK:\t"
#define TAG_RELEASED    "RELEASED:\t"

#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

#define DEBUG_MODE_HEADER "\033[1;37m"

#define DEBUG

#ifdef DEBUG
#define DBG_CLEAR(); std::cout << "\033[H\033[J";
#define DBG_COUT(dat, color); std::cout << color << dat << "\033[0m" << std::endl;
#else
#define DBG_CLEAR();
#define DBG_COUT(dat, color);
#endif

#define KA_STEP 1
#define KA_BASE 60 // 10*100ms
#define KA_EDGE 30


std::string getLocalHost();

bool isInConnectedClients(std::string client);

void makeMessage(char * buffer, const char * clientId, ControllerMsgState state);

void * SendThreadFunction(void * params);

void * ReceiveThreadFunction(void * params);

void * clientListThreadFunction(void * params);

void * StartSSDPControllerFunction(void * params);

void initSSDPController(std::string JSONFileName);

#endif
