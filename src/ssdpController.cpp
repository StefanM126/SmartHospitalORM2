#include "ssdpController.h"

static struct sockaddr_in addr_dest;
static struct sockaddr_in client_addr;
static unsigned int multi_server_sock;
static unsigned int multi_server_sock_reversed;

static int keepAlive = KA_BASE;
static char keepAliveChar[4];

static std::list < std::string > connectedClients;
static std::list < std::string > newConnectedClients;

static bool firstStart = true;
static std::mutex clientsListsMutex;
static std::mutex keepAliveMutex;

static ControllerClass validClients;

std::string getLocalHost() {
    char hostbuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;
    int hostname;
  
    // To retrieve hostname
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    if (hostname == -1) {
        perror("gethostname");
        exit(1);
    }

    strcat(hostbuffer, ".local");
  
    // To retrieve host information
    host_entry = gethostbyname(hostbuffer);
    if (host_entry == NULL) {
        perror("gethostbyname");
        exit(1);
    }
  
    // To convert an Internet network
    // address into ASCII string
    IPbuffer = inet_ntoa(*((struct in_addr*)
                           host_entry->h_addr_list[0]));
    if (IPbuffer == NULL) {
        perror("inet_ntoa");
        exit(1);
    }
  
    return std::string(IPbuffer);
}

bool isInConnectedClients(std::string client) {

    for (auto it = connectedClients.begin(); it != connectedClients.end(); it++) {
        if ( * it == client)
            return true;
    }

    return false;
}
void makeMessage(char * buffer,
    const char * clientId, ControllerMsgState state) {
    char data[BUFF_LEN] = {0};

    // Clear buffer
    for (int i = 0; i < BUFF_LEN; i++)
        buffer[i] = 0;

    // Make data
    strcpy(data, clientId);
    strcat(data, ":");
    strcat(data, CONTROLLER_NAME);
    strcat(data, ":");

    if (state == OK) {
        strcat(data, "OK");
        strcat(data, ":");
        strcat(data, getLocalHost().c_str());
        strcat(data, ":");

        sprintf(keepAliveChar, "%d", keepAlive);

        strcat(data, keepAliveChar);
    } else if (state == RLSD) {

        strcat(data, "RLSD");

    } else if (state == NOK) {
        strcat(data, "NOK");
    }

    // count length
    buffer[0] = strlen(data) + 1;

    // Add length
    strcat(buffer, data);
}

void * SendThreadFunction(void * params) {
    unsigned char buff_len = * (unsigned char * ) params;
    unsigned int addr_len = sizeof(addr_dest);
    char buffer[buff_len];

    memcpy(buffer, (char * ) params + 1, buff_len);
    if (sendto(multi_server_sock, buffer, buff_len, 0, (struct sockaddr * ) & addr_dest, addr_len) < 0) {
        perror("*** ERROR - sendto()\n");
        exit(1);
    }

    DBG_COUT(buffer, CYAN);

    return NULL;
}

void * ReceiveThreadFunction(void * params) {
    unsigned int addr_len = sizeof(client_addr);
    unsigned char buff_len;
    char buffer[BUFF_LEN] = {0};
    ClientMessage clientMsg;

    while (1) {
        if ((buff_len = recvfrom(multi_server_sock_reversed, buffer, sizeof(buffer), 0, (struct sockaddr * ) & client_addr, & addr_len)) < 0) {
            perror("*** ERROR - recvfrom() failed \n");
            exit(1);
        }

        bool regularMsg = clientMsg.parse(std::string(buffer));
        if (clientMsg.getReceiverId() == CONTROLLER_NAME) {
            pthread_t SendThread;

            if (regularMsg) {

                if (validClients.isInList(clientMsg.getType(), clientMsg.getPurpose())) {
                    DBG_COUT(buffer, YELLOW);
                    std::unique_lock < std::mutex > l(clientsListsMutex);

                    if (firstStart)
                        connectedClients.push_back(clientMsg.getSenderId());
                    else
                        newConnectedClients.push_back(clientMsg.getSenderId());

                    l.unlock();

                    makeMessage(buffer, clientMsg.getSenderId().c_str(), OK);
                    pthread_create( & SendThread, NULL, SendThreadFunction, buffer);
                } else {
                    DBG_COUT(buffer, RED);
                    makeMessage(buffer, clientMsg.getSenderId().c_str(), NOK);
                    pthread_create( & SendThread, NULL, SendThreadFunction, buffer);
                }

            } else {

                std::unique_lock < std::mutex > l(clientsListsMutex);
                bool clientIsInConectedList = isInConnectedClients(clientMsg.getSenderId());
                l.unlock();


                if (clientIsInConectedList) {
                    DBG_COUT(buffer, MAGENTA);
                    std::unique_lock < std::mutex > l(clientsListsMutex);
                    newConnectedClients.push_back(clientMsg.getSenderId());
                    l.unlock();

                } else {
                    std::cout << "\033[34m" << buffer << "\033[0m" << std::endl;
                    DBG_COUT(buffer, BLUE);
                    makeMessage(buffer, clientMsg.getSenderId().c_str(), RLSD);
                    pthread_create( & SendThread, NULL, SendThreadFunction, buffer);
                }
            }
        }
    }
    return NULL;
}

void * clientListThreadFunction(void * params) {
    while (1) {
        usleep(keepAlive * 1000 * 100);

        std::unique_lock < std::mutex > l(clientsListsMutex);

        if (firstStart) {
            firstStart = false;
        } else {
            connectedClients = newConnectedClients;
            newConnectedClients.clear();
        }

        if(connectedClients.size() > KA_EDGE)
            keepAlive = KA_BASE + KA_BASE/100*20;
        else
            keepAlive = KA_BASE;

        l.unlock();

    }
    return NULL;
}

void * StartSSDPControllerFunction(void * params) {
    pthread_t ReceiveThread;
    pthread_t clientListThread;

    pthread_create( & ReceiveThread, NULL, ReceiveThreadFunction, NULL);
    pthread_create( & clientListThread, NULL, clientListThreadFunction, NULL);

    pthread_join(ReceiveThread, NULL);

    close(multi_server_sock);
    close(multi_server_sock_reversed);

    return NULL;
}

void initSSDPController(std::string JSONFileName) {
    unsigned char TTL;
    struct ip_mreq mreq;
    int val;

    validClients = ControllerClass(JSON_PATH + JSONFileName);

    DBG_CLEAR();
    DBG_COUT("DEBUG MODE:", DEBUG_MODE_HEADER);
    DBG_COUT(validClients, BLUE);

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

    addr_dest.sin_family = AF_INET;
    addr_dest.sin_addr.s_addr = inet_addr(GROUP_ADDR);
    addr_dest.sin_port = htons(PORT_NUM);

    val = 1;
    if (setsockopt(multi_server_sock_reversed, SOL_SOCKET, SO_REUSEPORT, & val, sizeof(val)) < 0) {
        perror("*** ERROR - setsockopt()\n");
        exit(1);
    }

    if (bind(multi_server_sock_reversed, (struct sockaddr * ) & client_addr, sizeof(struct sockaddr)) < 0) {
        perror("*** ERROR - bind()\n");
        exit(1);
    }

    TTL = 1;
    if (setsockopt(multi_server_sock, IPPROTO_IP, IP_MULTICAST_TTL, (char * ) & TTL, sizeof(TTL)) < 0) {
        perror("*** ERROR - setsockopt()\n");
        exit(1);
    }

    if (setsockopt(multi_server_sock_reversed, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char * ) & mreq, sizeof(mreq)) < 0) {
        perror("*** ERROR - setsockopt()\n");
        exit(1);
    }
}
