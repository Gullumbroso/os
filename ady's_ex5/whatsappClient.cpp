#include <stdlib.h>
#include <regex>
#include "whatsappClient.h"

#define ERR_MSG "ERROR: "
#define NAME 1
#define ADDRESS 2
#define PORT 3

const std::regex nameRegex("[a-zA-Z0-9]+");

/**
 * initializes all the relevent data to start a connection with the server
 */
void whatsappClient::initConnection(char* name, char* address, char* port) {
    struct sockaddr_in addr;
    in_port_t port_num = 0;
    int sfd = 0;

    try {
        port_num = (in_port_t) stoul(port, nullptr, 10);
    } catch(...) {
        cerr << ERR_MSG << "stoul " << errno << endl;
        exit(EXIT_FAILURE);
    }
    addr.sin_port = htons(port_num);
    memset(addr.sin_zero,'\0',sizeof(addr.sin_zero));
    addr.sin_family = AF_INET;
    //connection establishment
    if(inet_aton(address, &(addr.sin_addr)) == 0 ||
       ((sfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)) {
        cerr << ERR_MSG << "socket " << errno << endl;
        exit(EXIT_FAILURE);
    }


    if(connect(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr)) < 0) {
        close(sfd);
        cerr << ERR_MSG << "connect " << errno << endl;
        exit(EXIT_FAILURE);
   }
    if (std::regex_match(name, nameRegex) == false){
        cerr << ERR_MSG << "Invalid input." << endl;
        close(sfd);
        exit(EXIT_FAILURE);
    }
    write(sfd, name, strlen(name));
    char message[256];
    ssize_t bytes_read = read(sfd, message, 256);
    message[bytes_read-1] = '\0';
    if ((string) message == "Client name is already in use."){
        cout << message << endl;
        close(sfd);
        exit(EXIT_FAILURE);
    }
    if ((string) message == "Failed to connect the server"){
        cout << message << endl;
        close(sfd);
        exit(EXIT_FAILURE);
    }
    socketFd = sfd;
}

/**
 * an infinite loop that waits until there's a message to read from the server
 */
void whatsappClient::loop() {
    while (true) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(socketFd, &readfds);
        timeval timer;
        timer.tv_sec = 1;
        timer.tv_usec = 0;
        int returnVal = select(FD_SETSIZE, &readfds, NULL, NULL, &timer);
        if (returnVal < 0) {
            cerr << ERR_MSG << "select " << errno << endl;
            close(socketFd);
            exit(EXIT_FAILURE);
        }
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            readUserInput();
        }
        if (FD_ISSET(socketFd, &readfds)) {
            readServerInput();
        }
    }
}

/**
 * Reads the input received from the server
 */
void whatsappClient::readServerInput() {
    char message[256];
    ssize_t bytes_read = read(socketFd, message, 256);
    message[bytes_read-1] = '\0';
    cout << message << endl;

}
/**
 * Makes sure the user input is valid. and if so it is sent to the server for furher operations
 */
void whatsappClient::readUserInput() {
    char message[256];
    ssize_t bytes_read = read(STDIN_FILENO, message, 256);
`    string msg = (string) message;
    if (msg.find("create_group") == 0){
        if (!checkCreateGroup(msg)){
            return;
        }
    }
    else if (msg.find("send") == 0){
        checkSendMsg(msg);
    }
    else if(msg.find("who") == 0){
        if (msg.size() > 4){
            cerr << ERR_MSG << "Invalid input." << endl;
            return;
        }
    }
    else if(msg.find("exit") == 0){
        if (msg.size() > 5){
            cerr << ERR_MSG << "Invalid input." << endl;
            return;
        }
        exitClient();
    }
    else if(msg.find("create_group") != 0 && msg.find("send") != 0 &&
            msg.find("who") != 0 && msg.find("exit") != 0){
        cerr << ERR_MSG << "Invalid input." << endl;
        return;
    }

    sendMsgToServer(message);

}

/**
    * a helper function that handles the flow of a client exiting the server
    */
void whatsappClient::exitClient(){
    string msg = "exit\n";
    write(socketFd, msg.c_str(), msg.length());
    readServerInput();
    close(socketFd);
    exit(0);

}

/**
 * write a message to the server
 * @param msg the message to be sent
 */
void whatsappClient::sendMsgToServer(string msg){
    write(socketFd, msg.c_str(), msg.length());
}

/**
 * checks if the create_group message that was written by the user is valid.
 * it checks if there are enough arguments, if the group's and the user's name is valid (matchs regex)
 * @param msg message that was written by the user
 */
bool whatsappClient::checkCreateGroup(string msg) {

    unsigned long firstSpace = msg.find(" ", 0);
    unsigned long secondSpace = msg.find(" ", firstSpace + 1);

    if(secondSpace == string::npos){
        // checking for enough arguments
        cerr << ERR_MSG << "Invalid input." << endl;
        return false;
    }
    unsigned long groupNameLen = secondSpace - (firstSpace + 1);
    string groupName = msg.substr(firstSpace + 1, groupNameLen); //finding the group's name
    if(!regex_match(groupName, nameRegex)){
        //chekcing if the group's name is valid
        cerr << ERR_MSG << "Invalid input." << endl;
        return false;
    }
    return true;
}


/**
 * checks if the send message that was written by the user is valid.
 * it checks if there are enough arguments, if the group's and the user's name is valid (matchs regex)
 * @param msg message that was written by the user
 */
void whatsappClient::checkSendMsg(string msg) {
    unsigned long firstSpace = msg.find(" ", 0);
    unsigned long secondSpace = msg.find(" ", firstSpace + 1);

    if(secondSpace == string::npos){
        // checking for enough arguments
        cerr << ERR_MSG << "Invalid input." << endl;
        return;
    }
    unsigned long nameLen = secondSpace - (firstSpace + 1);
    string name = msg.substr(firstSpace + 1, nameLen); //finding the group's name
    if(!regex_match(name, nameRegex)){
        //chekcing if the group's name is valid
        cerr << ERR_MSG << "Invalid input." << endl;
        return;
    }
}

/**
 * a main for the client
 * @param argc num of args
 * @param argv an array containing the name, address, port
 */
int main(int argc, char** argv) {
    if (argc != 4) {
        cout << "Usage: whasappClient clientName serverAddress serverPort" << endl;
        exit(EXIT_FAILURE);
    }
    whatsappClient c = whatsappClient();
    c.initConnection(argv[NAME],argv[ADDRESS],argv[PORT]);
    c.loop();
    exit(EXIT_SUCCESS);
}
