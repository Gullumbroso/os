//
// Created by Gilad Lumbroso on 06/20/2017.
//
#include <regex>
#include <stdlib.h>
#include "whatsappClient.h"

#define NAME 1
#define ADDRESS 2
#define PORT 3
#define FAILURE 1
#define SUCCESS 0
#define CHARS_SIZE 256
#define MAX_CLIENTS 10

const regex validName("[a-zA-Z0-9]+");


void whatsappClient::exitWithError(string msg , bool isCerr, int sfd)
{
    if(isCerr){
        cerr << ERR_MSG << msg << " " << errno << endl;
    }
    else{
        cout << msg << endl;
    }

    if(sfd != nullptr){
        close(sfd);
    }
    exit(FAILURE);
}

whatsappClient::whatsappClient(char *name, char *address, char *port) {
    initializeConnection(name, address, port);
}

void whatsappClient::initializeConnection(const char *name, const char *address, const char *port) {// initializations
    int socketFileDes = 0;
    in_port_t portNum = 0;
    struct sockaddr_in addr;

    try {
        portNum = (in_port_t) stoul(port, nullptr, MAX_CLIENTS);
    } catch(...) {
        exitWithError("stoul",true, nullptr);
    }

    // convert from host byte order to network byte order.
    addr.sin_port = htons(portNum);

    // Sets the bytes in addr.sin_zero to 0.
    memset(addr.sin_zero,'\0',sizeof(addr.sin_zero));
    addr.sin_family = AF_INET;
    int aton = inet_aton(address, &(addr.sin_addr));

    // creating an endpoint for communication
    socketFileDes = socket(PF_INET, SOCK_STREAM, 0);

    // if the adress is not valid, or could not create an endpoint for communication.
    if (aton == 0 || socketFileDes  < 0) {
        exitWithError("socket",true, nullptr);
    }

    // initiate a connection on a socket
    int res = connect(socketFileDes, (struct sockaddr *) &addr, sizeof(struct sockaddr));
    if(res < 0) {
        close(socketFileDes);
        exitWithError("connect",true, nullptr);
    }

    // ------- we are connected! -------

    // check if the input is valid
    if (!(regex_match(name, validName))){
        close(socketFileDes);
        exitWithError("Invalid input.",true, nullptr);
    }

    write(socketFileDes, name, strlen(name));
    char message[CHARS_SIZE];
    ssize_t bytes_read = read(socketFileDes, message, CHARS_SIZE);

    if (string(message) == "Client name is already in use."){
        exitWithError(message,false,socketFileDes);
    }
    if (string(message) == "Failed to connect the server"){
        exitWithError(message,false,socketFileDes);
    }
    clientSocket = socketFileDes;
}

/**
 * the loop of the client. after we initialize the connection we run it.
 */
void whatsappClient::runClient() {
    while (true) {
        fd_set readFdSocked;
        FD_ZERO(&readFdSocked);
        FD_SET(STDIN_FILENO, &readFdSocked);
        FD_SET(clientSocket, &readFdSocked);
        timeval timer;
        timer.tv_usec = 0;
        timer.tv_sec = 1;

        // allow our program to monitor multiple fds, waiting for the file descriptors to be "ready"
        int ret = select(FD_SETSIZE, &readFdSocked, NULL, NULL, &timer);
        if (ret < 0) {
            exitWithError("select",true,clientSocket);
        }

        // check where
        if (FD_ISSET(STDIN_FILENO, &readFdSocked)) {
            //check if user input is valid.
            sendUserInput();
        }
        if (FD_ISSET(clientSocket, &readFdSocked)) {
            // check if server input is valid.
            readServerInput();
        }
    }
}
void whatsappClient::readServerInput() {
    char msg[CHARS_SIZE];
    ssize_t bytes_read = read(clientSocket, msg, CHARS_SIZE);
    cout << msg << endl;
}

void whatsappClient::sendUserInput() {
    char message[CHARS_SIZE];
    ssize_t bytes_read = read(STDIN_FILENO, message, CHARS_SIZE);
    message[bytes_read] = '\0';
    string msg = (string) message;
    unsigned long firstSpace = msg.find(" ", 0);
    unsigned long secondSpace = msg.find(" ", firstSpace + 1);
    if (msg.find("create_group") == 0){
        if(!checkValidGroup(msg, firstSpace, secondSpace)){
            return;
        }
    }
    else if (msg.find("send") == 0){
        checkSend(msg, firstSpace, secondSpace);
    }

    else if(msg.find("who") == 0){
        if(!checkWho(msg)){
            return;
        }
    }
    else if(msg.find("exit") == 0){
        if(!checkExit(msg)){
            return;
        }
        string msg2 = "exit\n";
        write(clientSocket, msg2.c_str(), msg2.length());
        readServerInput();
        close(clientSocket);
        exit(0);
    }
    else if(msg.find("create_group") != 0 && msg.find("send") != 0 &&
            msg.find("who") != 0 && msg.find("exit") != 0){
        cerr << ERR_MSG << "Invalid input." << endl;
        return;
    }
    write(clientSocket, ((string) message).c_str(), ((string) message).length());
}

bool whatsappClient::checkExit(const string &msg) const {
    if (msg.size() > 5){
        cerr << ERR_MSG << "Invalid input." << endl;
        return false;
    }
    return true;
}

bool whatsappClient::checkWho(const string &msg) const {
    if (msg.size() > 4){
        cerr << ERR_MSG << "Invalid input." << endl;
        return false;
        }
    return true;
}

void whatsappClient::checkSend(const string &msg, unsigned long firstSpace, unsigned long secondSpace) const {
    if(secondSpace == basic_string::npos){
            // checking for enough arguments
        cerr << ERR_MSG << "Invalid input." << endl;
        return;
        }
    unsigned long nameLen = secondSpace - (firstSpace + 1);
    string name = msg.substr(firstSpace + 1, nameLen); //finding the group's name
    if(!regex_match(name, validName)){
            //chekcing if the group's name is valid
        cerr << ERR_MSG << "Invalid input." << endl;
        return;
        }
}

bool whatsappClient::checkValidGroup(const string &msg, unsigned long firstSpace, unsigned long secondSpace) const {
    if(secondSpace == basic_string::npos){
            // not valid arguments.
        cerr << ERR_MSG << "Invalid input." << endl;
        return false;
        }
    unsigned long groupNameLen = secondSpace - (firstSpace + 1);
    string groupName = msg.substr(firstSpace + 1, groupNameLen); //finding the group's name
    if(!regex_match(groupName, validName)){
            cerr << ERR_MSG << "Invalid input." << endl;
        return false;
        }
    return true;
}

/**
 * the main function of the client class
 * @param argc - number of arguments.
 * @param argv - list of the arguments.
 * @return 0 if success, 1 otherwise.
 */
int main(int argc, char* argv[]){
    if (argc != 4) {
        cout << "Usage: whasappClient clientName serverAddress serverPort" << endl;
        exit(FAILURE);
    }
    whatsappClient client = whatsappClient(argv[NAME],argv[ADDRESS], argv[PORT]);
    client.runClient();
    exit(SUCCESS);
}