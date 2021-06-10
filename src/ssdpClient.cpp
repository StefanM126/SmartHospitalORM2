#include "ssdpClient.h"

static struct sockaddr_in addr_dest;
static struct sockaddr_in client_addr;
static unsigned int multi_server_sock;
static unsigned int multi_server_sock_reversed;
static unsigned int multi_server_sock_keep_alive;

static pthread_t ReceiveThread;
static pthread_t SendThread;
static pthread_t KeepAliveThread;

static bool keepAliveStarted = false;
static int keepAlive;
static std::string clientName;
static ClientClass clientDescriptor;
static std::string mqttBrokerAddress;

static std::mutex mx;
static std::condition_variable cv;

void wait() {
    static unsigned frame = 1000000;
    unsigned us = rand() % frame + 1;
    usleep(us);

    if((frame *= 2) > MAX_FRAME_SIZE)
        frame = MAX_FRAME_SIZE;
}

void * KeepAliveThreadFunction(void * params) {

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    int keepAlive = * (int * ) params;
    unsigned int addr_len = sizeof(addr_dest);
    char str[] = CONTROLLER_NAME;
    std::string clientDescription = ":" + clientName + ":" + "Alive";
    strcat(str, clientDescription.c_str());
    unsigned char buff_len = strlen(str) + 1;
    unsigned char buffer[buff_len];
    memcpy(buffer, str, buff_len);

    while (1) {
        int wait = (keepAlive - rand()%(keepAlive/10 + 1)) * 1000 * 100;
        
        usleep(wait);
        DBG_COUT(buffer, MAGENTA);

        if (sendto(multi_server_sock_reversed, buffer, buff_len, 0, (struct sockaddr * ) & addr_dest, addr_len) < 0) {
            perror("*** ERROR - recvfrom() failed \n");
            exit(1);
        }
    }

    return NULL;
}

void * SendThreadFunction(void * params) {

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    unsigned int addr_len = sizeof(addr_dest);
    char str[] = CONTROLLER_NAME;
    std::string clientDescription = ":" + clientName + ":" + clientDescriptor.getType() + ":" + clientDescriptor.getAttribute();
    strcat(str, clientDescription.c_str());
    unsigned char buff_len = strlen(str) + 1;
    unsigned char buffer[buff_len];
    memcpy(buffer, str, buff_len);

    while (1) {
        DBG_COUT(buffer, YELLOW);

        if (sendto(multi_server_sock_reversed, buffer, buff_len, 0, (struct sockaddr * ) & addr_dest, addr_len) < 0) {
            perror("*** ERROR - recvfrom() failed \n");
            exit(1);
        }
        wait();
    }

    return NULL;
}

void * ReceiveThreadFunction(void * params) {
    unsigned int addr_len = sizeof(client_addr);
    unsigned char buff_len;
    char buffer[BUFF_LEN];
    ControllerMessage controllerMsg;
    std::string str;

    while (1) {
        if ((buff_len = recvfrom(multi_server_sock, buffer, sizeof(buffer), 0, (struct sockaddr * ) & client_addr, & addr_len)) < 0) {
            perror("*** ERROR - recvfrom() failed \n");
            exit(1);
        }

        buffer[buff_len] = 0;
        controllerMsg.parse(std::string(buffer));

        if (controllerMsg.getReceiverId() == clientName) {
            DBG_COUT(buffer, CYAN);

            if (controllerMsg.getState() == OK) {
                if (!keepAliveStarted) {
                    keepAliveStarted = true;
                    pthread_cancel(SendThread);

                    mqttBrokerAddress = controllerMsg.getBrokerIP();
                    cv.notify_one();
                    keepAlive = controllerMsg.getKeepAlive();
                    pthread_create( & KeepAliveThread, NULL, KeepAliveThreadFunction, & keepAlive);
                }
            } else if (controllerMsg.getState() == RLSD) {
                keepAliveStarted = false;
                pthread_cancel(KeepAliveThread);
                pthread_create( & SendThread, NULL, SendThreadFunction, NULL);
            }
        }
    }

    return NULL;
}

void * StartSSDPClientFunction(void * params) {
    pthread_create( & SendThread, NULL, SendThreadFunction, NULL);
    pthread_create( & ReceiveThread, NULL, ReceiveThreadFunction, NULL);

    pthread_join(ReceiveThread, NULL);
    pthread_join(SendThread, NULL);

    // Close and clean-up
    close(multi_server_sock);
    close(multi_server_sock_reversed);

    return NULL;
}

void initSSDPClient(std::string clientname, std::string clientJSONDesc) {
    unsigned char TTL;
    struct ip_mreq mreq;
    int val;

    clientName = clientname;
    clientDescriptor = ClientClass(JSON_PATH + clientJSONDesc);

    DBG_CLEAR();
    DBG_COUT("DEBUG MODE:", DEBUG_MODE_HEADER);
    DBG_COUT("ID: " + clientName, BLUE);
    DBG_COUT(clientDescriptor, BLUE);
    DBG_COUT("", BLUE);

 //============================== INITIALIZING ============================

    multi_server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    multi_server_sock_reversed = socket(AF_INET, SOCK_DGRAM, 0);
    multi_server_sock_keep_alive = socket(AF_INET, SOCK_DGRAM, 0);

    mreq.imr_multiaddr.s_addr = inet_addr(GROUP_ADDR);
    mreq.imr_interface.s_addr = INADDR_ANY;

    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = htons(PORT_NUM);

    addr_dest.sin_family = AF_INET;
    addr_dest.sin_addr.s_addr = inet_addr(GROUP_ADDR);
    addr_dest.sin_port = htons(PORT_NUM);

    val = 1;
    if (setsockopt(multi_server_sock, SOL_SOCKET, SO_REUSEPORT, & val, sizeof(val)) < 0) {
        perror("*** ERROR - setsockopt()\n");
        exit(1);
    }

    if (bind(multi_server_sock, (struct sockaddr * ) & client_addr, sizeof(struct sockaddr)) < 0) {
        perror("*** ERROR - bind()\n");
        exit(1);
    }

    if (setsockopt(multi_server_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char * ) & mreq, sizeof(mreq))) {
        perror("*** ERROR - setsockopt()\n");
        exit(1);
    }

    TTL = 1;
    if (setsockopt(multi_server_sock_reversed, IPPROTO_IP, IP_MULTICAST_TTL, (char * ) & TTL, sizeof(TTL)) < 0) {
        perror("*** ERROR - setsockopt()\n");
        exit(1);
    }

    if (setsockopt(multi_server_sock_keep_alive, IPPROTO_IP, IP_MULTICAST_TTL, (char * ) & TTL, sizeof(TTL)) < 0) {
        perror("*** ERROR - setsockopt()\n");
        exit(1);

    
    }
}


std::string getBrokerAddres() {
    std::unique_lock<std::mutex> lock(mx);
    cv.wait(lock);
    return mqttBrokerAddress;
}
