//
// Created by Gilad Lumbroso on 06/20/2017.
//

#ifndef UNTITLED1_WHATSAPPSERVER_H
#define UNTITLED1_WHATSAPPSERVER_H


#define WHO "who"
#define SEND "send"
#define CREATE_GROUP "create_group"
static const char *const CONNECT_SUCCESS = "Connected Successfully.\n";

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
public:
    int serverSocket;
    map<string, int> userToSocket;
    map<string, vector<string>> groupUsernames;


    whatsappServer(char* port);

    void clearResources();

    void exitWithError(string msg,int fd, bool toClose);

    void connection();

    void runServer();

    int initConnection(int &newConnection, fd_set &fds) const;

    void printSituation(string msg, string msg2);

    void tryToConnect(char *message, int newConnection);

    void commandParser(string userName, int fd);

    void group(string user, int fd, string msg);

    void sendMessage(string user, int fd, string msg);

    void checkConnections(string user, int fd);

    void clientExit(string user, int fd);

    void createGroup(string &user, int fd, const string &msg, size_t space2, const string &nameOfGroup);

    void selfSend(string &user, int fd, const string &name, const string &clientsMsg);
};


#endif //UNTITLED1_WHATSAPPSERVER_H


