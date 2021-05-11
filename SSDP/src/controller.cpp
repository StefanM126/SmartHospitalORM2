
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
#include <string.h>
#include <string>
#include "messageParser.h"
#include "jsonParser.h"


//----- Defines -------------------------------------------------------------
#define PORT_NUM        1900            
#define GROUP_ADDR      "239.255.255.250"
#define BUFF_LEN        256

#define CONTROLLER_NAME "Controller_1"
#define OK              "OK"
#define NOK             "NOK"
#define BROKER_IP       "192.168.1.1"
#define KEEP_ALIVE      "60"

static struct sockaddr_in   addr_dest;  
static struct sockaddr_in   client_addr;
static unsigned int         multi_server_sock;       
static unsigned int         multi_server_sock_reversed;

void makeMessage(char* buffer, const char* clientId, bool ok) {
    char  data[BUFF_LEN] = {0};
    
        std::cout << buffer << std::endl;
    // Clear buffer
    for(int i = 0; i < BUFF_LEN; i++)
        buffer[i] = 0;
    
    // Make data
    strcpy(data, clientId);
    strcat(data, ":");
    strcat(data, CONTROLLER_NAME);
    strcat(data, ":");

    if(ok) {
        strcat(data, OK);
        strcat(data, ":");
        strcat(data, BROKER_IP);
        strcat(data, ":");
        strcat(data, KEEP_ALIVE);
    }
    else {
        strcat(data, NOK);
    }
    
    // count length
    buffer[0] = strlen(data) + 1;
    
    // Add length
    strcat(buffer, data); 
}

void* SendThreadFunction(void* params){
    unsigned char buff_len = *(unsigned char*)params;
    unsigned int  addr_len = sizeof(addr_dest);
    char          buffer[buff_len];

    memcpy(buffer, (char*)params+1, buff_len);
    if(sendto(multi_server_sock, buffer, buff_len, 0, (struct sockaddr*)&addr_dest, addr_len) < 0) {
        perror("*** ERROR - sendto()\n");
        exit(1);
    }

    return NULL;
}

void* ReceiveThreadFunction(void* params) {
    unsigned int  addr_len = sizeof(client_addr);
    unsigned char buff_len;
    char          buffer[BUFF_LEN] = {0}; 
    std::string   str;
    char          data[BUFF_LEN] = {0};
    ClientMessage clientMsg;

    while(1) {
        if ((buff_len = recvfrom(multi_server_sock_reversed, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
              perror("*** ERROR - recvfrom() failed \n");
              exit(1);
          }

        clientMsg.parse(str.assign(buffer, buff_len));

        if (clientMsg.getReceiverId() == CONTROLLER_NAME) {
            pthread_t SendThread;
            //TODO potrebno proveriti da li uredjaj odgovara specifikacijama, bilo bi lepo fajl, json zvuci najlakse, ali ako imate boljih ideja time bolje!
            std::cout << buffer << std::endl;
            makeMessage(buffer, clientMsg.getSenderId().c_str(), true);
            // parameter = message length + message
            pthread_create(&SendThread, NULL, SendThreadFunction, buffer);
        }
    
    }
    return NULL;
}

int main(void) {
       
    unsigned char  TTL;                     
    struct ip_mreq mreq;            
    int            val;

    ClientClass("c.json");

// ============================= INITIALIZING ================================

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = htons(PORT_NUM);

    multi_server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    multi_server_sock_reversed = socket(AF_INET, SOCK_DGRAM, 0);
    mreq.imr_multiaddr.s_addr = inet_addr(GROUP_ADDR);
    mreq.imr_interface.s_addr = INADDR_ANY;

    addr_dest.sin_family = AF_INET;
    addr_dest.sin_addr.s_addr = inet_addr(GROUP_ADDR);
    addr_dest.sin_port = htons(PORT_NUM);

    addr_dest.sin_family = AF_INET; addr_dest.sin_addr.s_addr = inet_addr(GROUP_ADDR);
    addr_dest.sin_port = htons(PORT_NUM);

    val = 1;
    if (setsockopt(multi_server_sock_reversed, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val)) < 0) {
        perror("*** ERROR - setsockopt()\n");
        exit(1);
    }

    if (bind(multi_server_sock_reversed,(struct sockaddr *)&client_addr, sizeof(struct sockaddr)) < 0) {
        perror("*** ERROR - bind()\n");
        exit(1);
    }

    TTL = 1;
    if (setsockopt(multi_server_sock, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&TTL, sizeof(TTL)) < 0) {
      perror("*** ERROR - setsockopt()\n");
      exit(1);
    }

    if (setsockopt(multi_server_sock_reversed, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) < 0) {
      perror("*** ERROR - setsockopt()\n");
      exit(1);
    }

//=================================== THREADS ===================================

    pthread_t ReceiveThread;

    pthread_create(&ReceiveThread, NULL, ReceiveThreadFunction, NULL);

    pthread_join(ReceiveThread, NULL);

// =============================== CLOSING SOCKETS ==============================
    close(multi_server_sock);
    close(multi_server_sock_reversed);
    return 0;
}
