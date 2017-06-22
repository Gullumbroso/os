//
// Created by Gilad Lumbroso on 06/20/2017.
//

#ifndef UNTITLED1_WHATSAPPCLIENT_H
#define UNTITLED1_WHATSAPPCLIENT_H

#include <sys/socket.h>
#include "whatsappClient.h"
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <zconf.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <stdlib.h>
#include <libltdl/lt_system.h>

#define ERR_MSG "ERROR: "
#define FAILURE 1

using namespace std;

class whatsappClient
{
public:
    whatsappClient(char* name, char* address, char* port);

    int clientSocket;

    string username;

    void exitWithError(string msg, bool isCerr, int sfd);

    int callSocket(char *hostname, unsigned short portnum);

    void sendUserInput();

    void readServerInput();

    void runClient();

    void initializeConnection(const char *name, const char *address, const char *port);
};


#endif //UNTITLED1_WHATSAPPCLIENT_H
