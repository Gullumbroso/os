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

/**
 * represents whatsappClient who connect with the server.
 * send and recieve messages.
 */
class whatsappClient
{
public:

    int clientSocket;


    /**
     * the main constructor of the whatsappClient
     * @param name - the name of the client
     * @param address - the adress of the client
     * @param port - the port.
     * @return
     */
    whatsappClient(char* name, char* address, char* port);

    /**
     * function that help us to exit the program + cerr output.
     * @param msg - the msg to write.
     * @param isCerr - cerr or cout
     * @param socket - the socket to close
     */
    void exitWithError(string msg, bool isCerr, int sfd);

    /**
     * sends the input of the user. in case of errorrs - return.
     */
    void sendUserInput();

    /**
     * reads the server input.
     */
    void readServerInput();

    /**
     * the loop of the client, right after we initialize the connection
     */
    void runClient();

    /**
     * the initialization process of the connection.
     * @param name - the name of the client
     * @param address - the adress of the client
     * @param port - the port.
     */
    void initializeConnection(const char *name, const char *address, const char *port);

    /**
     * checks if the create group process is valid
     * @param msg - the msg we recieved from the client
     * @param space1
     * @param space2
     * @return true if it valid, false otherwise.
     */
    bool checkValidGroup(const string &msg, unsigned long space1, unsigned long space2) const;

    /**
     * check if there is a problem with sending the message to server.
     * @param msg - the message we want to send.
     * @param space1 - the first space in the string
     * @param space2 - the second space in the string
     */
    void checkSend(const string &msg, unsigned long space1, unsigned long space2) const;

    /**
     * checks if the who message is valid.
     * @param msg - the msg we got from the client.
     * @return
     */
    bool checkWho(const string &msg) const;

    /**
     * check if the exit is valid.
     * @param msg - the msg to check.
     * @return  true if it valid , false otherwise.
     */
    bool checkExit(const string &msg) const;
};


#endif //UNTITLED1_WHATSAPPCLIENT_H
