//
// Created by Gilad Lumbroso on 06/20/2017.
//

#ifndef UNTITLED1_WHATSAPPCLIENT_H
#define UNTITLED1_WHATSAPPCLIENT_H

#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <libltdl/lt_system.h>
#include <vector>
#include <string>

#define ERR_MSG "ERROR: "
#define FAILURE 1

using namespace std;

class whatsappClient
{

    string username;

    int socketAtServer;

    void exitWithError(string msg);

    int callSocket(char *hostname, unsigned short portnum);

    int readData(int s, char *buf, int n);
};


#endif //UNTITLED1_WHATSAPPCLIENT_H
