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

/**
 * the whatsappServer class who connect with clients.
 * send and recieve messages.
 */
class whatsappServer {
public:
    int serverSocket;
    map<string, int> userToSocket;
    map<string, vector<string>> groupUserNames;

    /**
     * the main constructor of the whatsappServer
     * @param port - the port we use.
     * @return
     */
    whatsappServer(char *port);

    /**
     * clears all the resources.
     */
    void clearResources();

    /**
     * function that help us to exit the connection well.
     * @param msg - the message we recieve to print to cerr.
     * @param fd - the fd of the socket.
     * @param toClose - if there is need to close the socket of the client.
     */
    void exitWithError(string msg, int fd, bool toClose);

    /**
     * try to open new connection.
     */
    void connection();

    /**
     * loop of the server. try to recieve and send messages and treat them.
     * @return
     */
    void runServer();

    /**
     * init new connection of the server.
     * @param newConnection - the new connection
     * @param fds - fd of the socket.
     * @return
     */
    int initConnection(int &newConnection, fd_set &fds) const;

    /**
     * print the message and the information on it.
     */
    void printSituation(string msg, string msg2);

    /**
     * checks if the new socket is already in the set, if not - add.
     * @param message - the message we got for connection.
     * @param newConnection - the fd of the new connection
     */
    void tryToConnect(char *message, int newConnection);

    /**
     * parser of the command we got from the client.
     * @param userName - the client who wrote the command.
     * @param fd - the file descriptor of the socket.
     */
    void commandParser(string userName, int fd);

    /**
     * when we decided that group is the current operation
     * @param user - the user who want to do that operation
     * @param fd - the fd of the socket.
     * @param msg - the message that the server got.
     */
    void group(string user, int fd, string msg);

    /**
     * if we decided that the server should send a message
     * @param user - the user who sent the message
     * @param fd - the fd of the socket
     * @param msg - the message
     */
    void sendMessage(string user, int fd, string msg);

    /**
     * checks the connections we have. in case the client wrote "who"
     * @param user - the user who want to check.
     * @param fd - the fd of the socket.
     */
    void checkConnections(string user, int fd);

    /**
     * in case the client send message to exit from the whatsappServer
     * @param user - the user who wants to disconnect.
     * @param fd - file descriptor of the socket.
     */
    void clientExit(string user, int fd);

    /**
     * in case a user want to create group.
     * @param user - the user who want to create the group.
     * @param fd - the fd of the socket.
     * @param msg - the msg we got from the user.
     * @param space2 - the second space in the msg.
     * @param nameOfGroup - the name of the group we want to open.
     */
    void createGroup(string &user, int fd, const string &msg, size_t space2, const string &nameOfGroup);

    /**
     * in case we send a message to ourselve.
     * @param user - the user who send the message
     * @param fd - file descriptor of the socket.
     * @param name - the name of the user we want to send to.
     * @param clientsMsg - the msg.
     */
    void selfSend(string &user, int fd, const string &name, const string &clientsMsg);
};


#endif //UNTITLED1_WHATSAPPSERVER_H


