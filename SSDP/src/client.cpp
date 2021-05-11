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

#include "messageParser.h"
#include "jsonParser.h"


//----- Defines -------------------------------------------------------------
#define PORT_NUM    1900          
#define GROUP_ADDR "239.255.255.250"        

#define CLIENT_NAME     "Client_1"
#define CONTROLLER_NAME "Controller_1"
#define BUFF_LEN        256 


static struct sockaddr_in   addr_dest;      
static struct sockaddr_in   client_addr; 
static unsigned int   multi_server_sock; 
static unsigned int   multi_server_sock_reversed;


void* ReceiveThreadFunction(void* params) {
    unsigned int  addr_len = sizeof(client_addr);
    unsigned char buff_len; 
    char buffer[BUFF_LEN];       
    ControllerMessage controllerMsg;
    std::string str;

    while(1) {
        if ((buff_len = recvfrom(multi_server_sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
              perror("*** ERROR - recvfrom() failed \n");
              exit(1);
          }

        buffer[buff_len] = 0;
        controllerMsg.parse(str.assign(buffer, buff_len));

        if(controllerMsg.getReceiverId() == CLIENT_NAME) {
            // TODO ako udje ovde poziva se send thread nakon nasumicnog broja sekundi manjeg od keep alive
            // HINT: pogledati klase! Keep alive ce biti int
            std::cout << buffer << std::endl;
        }
    }

    return NULL;
}

void* SendThreadFunction(void* params) {
    unsigned int  addr_len = sizeof(addr_dest);
    char str[] = CONTROLLER_NAME; 
    strcat(str, ":Client_1:Aktuator:Osvetljenje");
    //TODO id kao i informacije o klinetu treba da budu modularne, i da sa svakim novim pokretanjem dobija nov klijent
    unsigned char buff_len =  strlen(str) + 1; 
    unsigned char buffer[buff_len];
    memcpy(buffer, str, buff_len); 

    while(1) {
        if (sendto(multi_server_sock_reversed, buffer, buff_len, 0, (struct sockaddr*)&addr_dest, addr_len) < 0) {
              perror("*** ERROR - recvfrom() failed \n");
              exit(1);
          }
        
        printf("%s\n",buffer);
        sleep(2); // ovo skinuti kad se namesti nasumično
    }

    return NULL;
}

int main(void) {
     
    unsigned char  TTL;
    struct ip_mreq mreq;              
    int            val;
    
    ClientClass("c");

//============================== INITIALIZING ============================

    multi_server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    multi_server_sock_reversed = socket(AF_INET, SOCK_DGRAM, 0);
    mreq.imr_multiaddr.s_addr = inet_addr(GROUP_ADDR);
    mreq.imr_interface.s_addr = INADDR_ANY;

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY; client_addr.sin_port = htons(PORT_NUM);
    
    addr_dest.sin_family = AF_INET;
    addr_dest.sin_addr.s_addr = inet_addr(GROUP_ADDR);
    addr_dest.sin_port = htons(PORT_NUM);

    val = 1;
    if (setsockopt(multi_server_sock, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val)) < 0) {
        perror("*** ERROR - setsockopt()\n");
        exit(1);
    } 
 
    if (bind(multi_server_sock,(struct sockaddr *)&client_addr, sizeof(struct sockaddr)) < 0) {
        perror("*** ERROR - bind()\n");
        exit(1);
    }

    if (setsockopt(multi_server_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq))) {
        perror("*** ERROR - setsockopt()\n");
        exit(1);
    }

    TTL = 1;
    if (setsockopt(multi_server_sock_reversed, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&TTL, sizeof(TTL)) < 0) {
      perror("*** ERROR - setsockopt()\n");
      exit(1);
    }

//========================== THREADS =======================================

    pthread_t ReceiveThread;
    pthread_t SendThread;

    pthread_create(&ReceiveThread, NULL, ReceiveThreadFunction, NULL);
    pthread_create(&SendThread, NULL, SendThreadFunction, NULL);

    pthread_join(ReceiveThread, NULL);
    pthread_join(SendThread, NULL);

    // Close and clean-up
    close(multi_server_sock);
    close(multi_server_sock_reversed);
    return 0;
}
