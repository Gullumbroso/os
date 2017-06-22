#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <libltdl/lt_system.h>
#include <vector>
#include "whatsappServer.h"

#define ERR_MSG "ERROR: "
#define FAILURE -1
#define CONNECTION_LIMIT 10
#define EXIT_VAL "exit"
#define PORT_NUM 1
#define ERR_VAL -1

const std::regex groupNameRegex("[a-zA-Z0-9]+");

/**
    * initializes the server
    */
int whatsappServer::initServer(char *port) {

    //first step
    int portNumber = stoi(port);
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd < 0){
        cerr << ERR_MSG << "socket " << errno << endl;//check if fprintf or reg print
        exit(FAILURE);
    }
    sockaddr_in sa;
    memset(&sa,'\0', sizeof(sockaddr_in));
    sa.sin_family = AF_INET;
    sa.sin_port = htons((uint16_t)portNumber);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    //second step
    int returnVal = bind(serverfd, (sockaddr*)&sa, sizeof(sa));
    if (returnVal < 0){
        cerr << ERR_MSG << "bind " << errno << endl;
        exit(FAILURE);
    }

    //third step
    returnVal = listen(serverfd, CONNECTION_LIMIT);
    if (returnVal < 0){
        cerr << ERR_MSG << "listen " << errno << endl;
        exit(FAILURE);
    }
    this->serverFd = serverfd;
    return serverfd;
}

/**
 * an infinite loop that waits until there's a message to read from the clients
 */
void whatsappServer::loop(){
    while (true){
        fd_set readfds;
        FD_ZERO(&readfds);
        for (auto pair : usernameToFd){
            FD_SET(pair.second, &readfds);
        }
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(serverFd, &readfds);
        timeval timer;
        timer.tv_sec = 1;
        timer.tv_usec = 0;

        int returnVal = select(FD_SETSIZE, &readfds, NULL, NULL, &timer);
        if(returnVal < 0){
            cerr << ERR_MSG << "select " << errno << endl;
            for (auto pair : usernameToFd){
                close(pair.second);
            }
            close(serverFd);
            exit(FAILURE);
        }
        if (FD_ISSET(STDIN_FILENO, &readfds)){
            isExit();
        }
        if (FD_ISSET(serverFd, &readfds)){
            recieveConnection();
        }
        for (auto pair : usernameToFd) {
            if (FD_ISSET(pair.second, &readfds)) {
                dealWithCommand(pair.first, pair.second);
            }
        }
    }
}

/**
 * write a message to the user's console
 * @param fd of the user to write to
 * @param msg the message to write
 */
void writeMsgToUser(int fd, string msg){
    write(fd, msg.c_str(), msg.length());
}

/**
 * deals with commands received from the cilents. seperate according to the first word and scalls the relevent function
 * that handles the command
 * @param user who send the comman
 * @param fd of the user
 */
void whatsappServer::dealWithCommand(string user, int fd){
    char message[256];
    ssize_t bytes_read = read(fd, message, 256);
    message[bytes_read] = '\0';
    string msg = (string) message;
    if (msg.find("create_group") == 0){
        createGroup(user, fd, msg);
    }
    else if (msg.find("send") == 0){
        sendMsg(user, fd, msg);
    }
    else if(msg.find("who") == 0){
        whoisConnected(user, fd);
    }
    else if(msg.find("exit") == 0){
        clientExit(user, fd);
    }
}

/**
 * handles the creation of a new group. including verifying relevent things
 */
void whatsappServer::createGroup (string user, int fd, string msg){
    unsigned long firstSpace = msg.find(" ", 0);
    unsigned long secondSpace = msg.find(" ", firstSpace + 1);
    unsigned long groupNameLen = secondSpace - (firstSpace + 1);
    string groupName = msg.substr(firstSpace + 1, groupNameLen); //finding the group's name

    if (usernameToFd.find(groupName) != usernameToFd.end() ||
            groupToUsernames.find(groupName) != groupToUsernames.end()){
        cout << user << ": ERROR: failed to create group \"" << groupName << "\"." << endl;
        string msgToClient = "ERROR: failed to create group \"" + groupName + "\".\n";
        writeMsgToUser(fd, msgToClient);
    }
    else {
        string clients = msg.substr(secondSpace + 1);
        vector<string> clientsNames;
        unsigned long commaIdx = clients.find(",");
        while (commaIdx != string::npos) { //adding all the clients to the clients vector after splitting the clients
            //list by commas
            string client = clients.substr(0, commaIdx);
            if ((find(clientsNames.begin(), clientsNames.end(), client)) == clientsNames.end()) {
                if (usernameToFd.find(client) != usernameToFd.end()) {
                    clientsNames.push_back(client);
                }
            }
            clients = clients.substr(commaIdx + 1);
            commaIdx = clients.find(",");
        }
        //adding the user who created the group if he didn't appear in the group clients
        if ((find(clientsNames.begin(), clientsNames.end(), user)) == clientsNames.end()) {
            clientsNames.push_back(user);
        }
        //adding the last client, not separated by comma
        clients.pop_back();
        if ((find(clientsNames.begin(), clientsNames.end(), clients)) == clientsNames.end()) {
            if (usernameToFd.find(clients) != usernameToFd.end()) {
                clientsNames.push_back(clients);
            }
        }
        if (clientsNames.size() < 2) { //checking if the group only has one member (the user itself)
            cout << user << ": ERROR: failed to create group \"" << groupName << "\"." << endl;
            string msgToClient = "ERROR: failed to create group \"" + groupName + "\".\n";
            writeMsgToUser(fd, msgToClient);
        }
        else {
            groupToUsernames[groupName] = clientsNames; //adding the group and its members to the map.

            cout << user << ": Group \"" << groupName << "\" was created successfully." << endl;
            string msgToClient = "Group \"" + groupName + "\" was created successfully.\n";
            writeMsgToUser(fd, msgToClient);
        }
    }
}

/**
* manages the creation of a new connectio once a client tries to connect to
* the server
*/
void whatsappServer::recieveConnection(){
    char message[256];
    int newConnection;
    newConnection = accept(serverFd,NULL,NULL);
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(newConnection, &readfds);
    timeval timer;
    timer.tv_sec = 2;
    timer.tv_usec = 0;
    int returnVal = select(FD_SETSIZE, &readfds, NULL, NULL, &timer);
    if (returnVal == ERR_VAL){
        cerr << ERR_MSG << "select " << errno << endl;
        close(newConnection);//todo close all the connections
        exit(FAILURE);
    }
    if (FD_ISSET(newConnection, &readfds)) {
        ssize_t bytes_read = read(newConnection, message, 256);
        message[bytes_read] = '\0';
        if (usernameToFd.find(message) == usernameToFd.end()) {
            usernameToFd[message] = newConnection;
            write(newConnection, "Connected Successfully.\n", 24);
            cout << message << " connected." << endl;
            return;
        }
        else{
            string msgToClient = "Client name is already in use.\n";
            write(newConnection, msgToClient.c_str(), msgToClient.length());
            cout << message << " failed to connect." << endl;
            close(newConnection);
            return;
        }
    }
    string msgToClient = "Failed to connect the server\n";
    write(newConnection, msgToClient.c_str(), msgToClient.length());
    cout << message << " failed to connect." << endl;
    close(newConnection);

}


    /**
    * checks if the stdin input is EXIT
    */
void whatsappServer::isExit() {
    string input;
    getline(cin, input);
    transform(input.begin(), input.end(), input.begin(), ::tolower);
    if (input.compare(EXIT_VAL) == 0){
        for (auto pair : usernameToFd){
            close(pair.second);
        }
        close(serverFd);
        exit(0);
    }

}

/**
* In charge of sending a message from a client to another.
* is called upon the client's "send" command
*/
void whatsappServer::sendMsg(string user, int fd, string msg) {
    unsigned long firstSpace = msg.find(" ", 0);
    unsigned long secondSpace = msg.find(" ", firstSpace + 1);
    unsigned long nameLen = secondSpace - (firstSpace + 1);
    string name = msg.substr(firstSpace + 1, nameLen);
    string clientsMsg = msg.substr(secondSpace + 1);
    clientsMsg.pop_back();
    if (user == name){ //the user tried to send himself a msg
        cout << user << ": ERROR: failed to send \"" << clientsMsg << "\" to " << name << "." << endl;
        writeMsgToUser(fd, "ERROR: failed to send.\n");
        return;
    }
    string msgToSend = user + ": " + clientsMsg + "\n";
    //checks if its a user name
    if ((usernameToFd.find(name) != usernameToFd.end()) && (name != user)){
        int nameFd = usernameToFd[name];
        writeMsgToUser(nameFd, msgToSend);
        cout << user << ": \"" << clientsMsg << "\" was sent successfully to " << name << "." << endl;
        writeMsgToUser(fd, "Sent successfully.\n");
    }
    //checks if its a group name
    else if (groupToUsernames.find(name) != groupToUsernames.end()) {
        vector<string> groupMembers = groupToUsernames[name];
        //checks if the user is in this group
        if (find(groupMembers.begin(), groupMembers.end(), user) != groupMembers.end()) {
            for (unsigned int i = 0; i < groupMembers.size(); i++) {
                int nameFd = usernameToFd[groupMembers[i]];
                if (nameFd != fd) {
                    writeMsgToUser(nameFd, msgToSend);
                }
            }
            cout << user << ": \"" << clientsMsg << "\" was sent successfully to " << name << "." << endl;
            writeMsgToUser(fd, "Sent successfully.\n");
        }
        else{ //the user isn't in this group
            cout << user << ": ERROR: failed to send \"" << clientsMsg << "\" to " << name << "." << endl;
            writeMsgToUser(fd, "ERROR: failed to send.\n");
        }
    }
    else{ //no user or group with this name
        cout << user << ": ERROR: failed to send \"" << clientsMsg << "\" to " << name << "." << endl;
        writeMsgToUser(fd, "ERROR: failed to send.\n");
    }
}
/**
* In charge of printing to the client the list of clients who are connected to
* the server at that moment. is called upon the client's "who" command
*/
void whatsappServer::whoisConnected(string user, int fd) {
    vector<string> connectedUsers;
    if (usernameToFd.size() == 0){ //checks if empty
        writeMsgToUser(fd, ""); //if so, will send an empty string
        cout << user << ": Requests the currently connected client names." << endl;
    }
    else {
        for (map<string, int>::iterator it = usernameToFd.begin(); it != usernameToFd.end(); ++it) {
            connectedUsers.push_back(it->first);
        }
        sort(connectedUsers.begin(), connectedUsers.end()); //sorts the names
        string result = connectedUsers[0];
        for (unsigned int i = 1; i < connectedUsers.size(); ++i) { //creating a string of names separated by commas
            result += "," + connectedUsers[i];
        }
        result = result + ".\n";
        writeMsgToUser(fd, result);
        cout << user << ": Requests the currently connected client names." << endl;
    }
}
/**
* In charge of letting the client exit the chat and the connection successfully.
* is called upon the client's "exit" command
*/
void whatsappServer::clientExit(string user, int fd) {
    //for loop on all the different groups, checking if the user is there
    for (map<string, vector<string>>::iterator it = groupToUsernames.begin(); it != groupToUsernames.end(); ++it) {
        vector<string> usersInGroup = it->second; //the users vector in the group
        vector<string>::iterator thisUserIter = find(usersInGroup.begin(), usersInGroup.end(), user);
        if (thisUserIter != usersInGroup.end()){ //checking if the user is in this group
            usersInGroup.erase(thisUserIter); //if so- erase it
        }
    }
    usernameToFd.erase(user);
    writeMsgToUser(fd, "Unregistered successfully.\n");
    close(fd);
    cout << user << ": Unregistered successfully." << endl;
}

/**
* a main
*/
int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "Usage: whatsappServer portNum" << endl;
        exit(EXIT_FAILURE);
    }
    whatsappServer s = whatsappServer();
    s.initServer(argv[PORT_NUM]);
    s.loop();
    exit(EXIT_SUCCESS);
}
