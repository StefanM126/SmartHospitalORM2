#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

//----- Include files -------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <string>
#include <string.h>
#include <mutex>
#include <condition_variable>
#include "messageParser.h"
#include "jsonParser.h"


//----- Defines -------------------------------------------------------------
#define PORT_NUM 1900
#define GROUP_ADDR "239.255.255.250"

// #define CLIENT_NAME     "Client_1"
#define CONTROLLER_NAME "Controller_1"
#define BUFF_LEN 256

#define JSON_PATH std::string("../json/")
#define MAX_FRAME_SIZE 8000000

#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

#define DEBUG_MODE_HEADER "\033[1;37m"

//#define DEBUG

#ifdef DEBUG
#define DBG_CLEAR(); std::cout << "\033[H\033[J";
#define DBG_COUT(dat, color); std::cout << color << dat << "\033[0m" << std::endl;
#else
#define DBG_CLEAR();
#define DBG_COUT(dat, color);
#endif

void wait();
void * KeepAliveThreadFunction(void * params); 
void * SendThreadFunction(void * params); 
void * ReceiveThreadFunction(void * params);
void * StartSSDPClientFunction(void * params);
void initSSDPClient(std::string clientname, std::string clientJSONDesc);
std::string getBrokerAddres();

#endif
