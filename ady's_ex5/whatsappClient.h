//
// Created by ady.kaiser on 6/13/17.
//

#ifndef EX5_WHATSAPPCLIENT_H
#define EX5_WHATSAPPCLIENT_H
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
using namespace std;
/**
 * a class representing a client using the whatsappServer
 */
class whatsappClient {
private:
    int socketFd;
public:

/**
 * initializes all the relevent data to start a connection with the server
 */
    void initConnection(char* name, char* address, char* port);

    /**
  * an infinite loop that waits until there's a message to read from the server
  */
    void loop();

    /**
 * Makes sure the user input is valid. and if so it is sent to the server for furher operations
 */
    void readUserInput();
    /**
 * Reads the input received from the server
 */
    void readServerInput();
    /**
 * write a message to the server
 * @param msg the message to be sent
 */
    void sendMsgToServer(string msg);

/**
 * checks if the create_group message that was written by the user is valid.
 * it checks if there are enough arguments, if the group's and the user's name is valid (matchs regex)
 * @param msg message that was written by the user
 */
    bool checkCreateGroup(string msg);
/**
 * checks if the send message that was written by the user is valid.
 * it checks if there are enough arguments, if the group's and the user's name is valid (matchs regex)
 * @param msg message that was written by the user
 */
    void checkSendMsg(string msg);

    /**
     * a helper function that handles the flow of a client exiting the server
     */
    void exitClient();
};


#endif //EX5_WHATSAPPCLIENT_H
