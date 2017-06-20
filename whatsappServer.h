//
// Created by Gilad Lumbroso on 06/20/2017.
//

#ifndef UNTITLED1_WHATSAPPSERVER_H
#define UNTITLED1_WHATSAPPSERVER_H


#include <netinet/in.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <libltdl/lt_system.h>
#include <vector>
#include <map>
#include <string>

using namespace std;


class whatsappServer
{
    int serverSocket;
    map<string, int> userToSocket;

    whatsappServer(char* port);

    void exitWithError(string msg);

    int getConnection();

    int runServer();
};


#endif //UNTITLED1_WHATSAPPSERVER_H


