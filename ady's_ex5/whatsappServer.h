//
// Created by ady.kaiser on 6/13/17.
//

#ifndef EX5_WHATSAPPSERVER_H
#define EX5_WHATSAPPSERVER_H
#include <iostream>
#include <map>
#include <regex>


using namespace std;

/**
* A class implementing the whatsapp server.
*/
class whatsappServer {
private:
    int serverFd;
    map<string,int> usernameToFd;
    map<string, vector<string>> groupToUsernames;

public:
    /**
     * dflt c-tor
     */
    whatsappServer(){}
    /**
   * dflt d-tor
   */
    ~whatsappServer(){}
    /**
    * initializes the server
    */
    int initServer(char* port);
    /**
    * checks if the stdin input is EXIT
    */
    void isExit();
    /**
     * an infinite loop that waits until there's a message to read from the clients
     */
    void loop();
    /**
    * manages the creation of a new connectio once a client tries to connect to
    * the server
    */
    void recieveConnection();
    /**
     * deals with commands received from the cilents. seperate according to the first word and scalls the relevent function
     * that handles the command
     * @param user who send the comman
     * @param fd of the user
     */
    void dealWithCommand(string user, int fd);

    /**
     * handles the creation of a new group. including verifying relevent things
     */
    void createGroup(string user, int fd, string msg);
    /**
    * In charge of sending a message from a client to another.
    * is called upon the client's "send" command
    */
    void sendMsg(string user, int fd, string msg);
    /**
    * In charge of printing to the client the list of clients who are connected to
    * the server at that moment. is called upon the client's "who" command
    */
    void whoisConnected(string user, int fd);
    /**
    * In charge of letting the client exit the chat and the connection successfully.
    * is called upon the client's "exit" command
    */
    void clientExit(string user, int fd);

};


#endif //EX5_WHATSAPPSERVER_H
