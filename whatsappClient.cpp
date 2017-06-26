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

/**
 * function that help us to exit the program + cerr output.
 * @param msg - the msg to write.
 * @param isCerr - cerr or cout
 * @param socket - the socket to close
 */
void whatsappClient::exitWithError(string msg, bool isCerr, int socket) {
    if (isCerr) {
        cerr << ERR_MSG << msg << " " << errno << endl;
    } else {
        cout << msg << endl;
    }
    if (socket > 0) {
        close(socket);
    }
    exit(FAILURE);
}
/**
 * the main constructor of the whatsappClient
 * @param name - the name of the client
 * @param address - the adress of the client
 * @param port - the port.
 * @return
 */
whatsappClient::whatsappClient(char *name, char *address, char *port) {
    initializeConnection(name, address, port);
}

/**
 * the initialization process of the connection.
 * @param name - the name of the client
 * @param address - the adress of the client
 * @param port - the port.
 */
void whatsappClient::initializeConnection(const char *name, const char *address,
                                          const char *port) {// initializations
    int socketFileDes = 0;
    in_port_t portNum = 0;
    struct sockaddr_in addr;

    try {
        portNum = (in_port_t) stoul(port, nullptr, MAX_CLIENTS);
    } catch (...) {
        exitWithError("stoul", true, -1);
    }

    // convert from host byte order to network byte order.
    addr.sin_port = htons(portNum);

    // Sets the bytes in addr.sin_zero to 0.
    memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));
    addr.sin_family = AF_INET;
    int aton = inet_aton(address, &(addr.sin_addr));

    // creating an endpoint for communication
    socketFileDes = socket(PF_INET, SOCK_STREAM, 0);

    // if the adress is not valid, or could not create an endpoint for communication.
    if (aton == 0 || socketFileDes < 0) {
        exitWithError("socket", true, -1);
    }

    // initiate a connection on a socket
    int res = connect(socketFileDes, (struct sockaddr *) &addr, sizeof(struct sockaddr));
    if (res < 0) {
        close(socketFileDes);
        exitWithError("connect", true, -1);
    }

    // ------- we are connected! -------

    // check if the input is valid
    if (!(regex_match(name, validName))) {
        close(socketFileDes);
        exitWithError("Invalid input.", true, -1);
    }

    write(socketFileDes, name, strlen(name));
    char msg1[CHARS_SIZE];
    ssize_t bytes_read = read(socketFileDes, msg1, CHARS_SIZE);
    msg1[bytes_read-1] = '\0';
    if (bytes_read < 0) {
        exitWithError("read ", true, socketFileDes);
    }

    if (string(msg1) == "Client name is already in use.") {
        exitWithError(msg1, false, socketFileDes);
    }
    if (string(msg1) == "Failed to connect the server") {
        exitWithError(msg1, false, socketFileDes);
    }
    clientSocket = socketFileDes;
    cout << "Connected Successfully.\n";
}

/**
 * the loop of the client, right after we initialize the connection
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
            exitWithError("select", true, clientSocket);
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

/**
 * reads the server input.
 */
void whatsappClient::readServerInput() {
    char msg[CHARS_SIZE];
    ssize_t bytes_read = read(clientSocket, msg, CHARS_SIZE);
    msg[bytes_read-1] = '\0';
    if (bytes_read < 0) {
        exitWithError("read ", true, clientSocket);
    }
    cout << msg << endl;
}

/**
 * sends the input of the user. in case of errorrs - return.
 */
void whatsappClient::sendUserInput() {
    char msg[CHARS_SIZE];
    ssize_t bytes_read = read(STDIN_FILENO, msg, CHARS_SIZE);
    msg[bytes_read] = '\0';
    string msg1 = (string) msg;
    unsigned long firstSpace = msg1.find(" ", 0);
    unsigned long secondSpace = msg1.find(" ", firstSpace + 1);
    if (msg1.find("create_group") == 0) {
        if (!checkValidGroup(msg1, firstSpace, secondSpace)) {
            return;
        }
    } else if (msg1.find("send") == 0) {
        checkSend(msg1, firstSpace, secondSpace);
    } else if (msg1.find("who") == 0) {
        if (!checkWho(msg1)) {
            return;
        }
    } else if (msg1.find("exit") == 0) {
        if (!checkExit(msg1)) {
            return;
        }
        string msg2 = "exit\n";
        write(clientSocket, msg2.c_str(), msg2.length());
        readServerInput();
        close(clientSocket);
        exit(0);
    } else if (msg1.find("create_group") != 0 && msg1.find("send") != 0 &&
               msg1.find("who") != 0 && msg1.find("exit") != 0) {
        cerr << ERR_MSG << "Invalid input." << endl;
        return;
    }
    write(clientSocket, ((string) msg).c_str(), ((string) msg).length());
}

/**
 * check if the exit is valid.
 * @param msg - the msg to check.
 * @return  true if it valid , false otherwise.
 */
bool whatsappClient::checkExit(const string &msg) const {
    if (msg.size() > 5) {
        cerr << ERR_MSG << "Invalid input." << endl;
        return false;
    }
    return true;
}

/**
 * checks if the who string is valid.
 * @param msg - the msg we got from the client.
 * @return
 */
bool whatsappClient::checkWho(const string &msg) const {
    if (msg.size() > 4) {
        cerr << ERR_MSG << "failed to receive list of connected clients." << endl;
        return false;
    }
    return true;
}

/**
 * check if there is a problem with sending the message to server.
 * @param msg - the message we want to send.
 * @param space1 - the first space in the string
 * @param space2 - the second space in the string
 */
void whatsappClient::checkSend(const string &msg, unsigned long space1,
                               unsigned long space2) const {
    if (space2 == string::npos) {
        // checking for enough arguments
        cerr << ERR_MSG << "failed to send." << endl;
        return;
    }
    unsigned long nameLen = space2 - (space1 + 1);
    string name = msg.substr(space1 + 1, nameLen); //finding the group's name
    if (!regex_match(name, validName)) {
        //chekcing if the group's name is valid
        cerr << ERR_MSG << "Invalid input." << endl;
        return;
    }
}

/**
 * checks if the create group process is valid
 * @param msg - the msg we recieved from the client
 * @param space1
 * @param space2
 * @return true if it valid, false otherwise.
 */
bool whatsappClient::checkValidGroup(const string &msg, unsigned long space1,
                                     unsigned long space2) const {
    if (space2 == string::npos) {
        // not valid arguments.
        cerr << ERR_MSG << "Invalid input." << endl;
        return false;
    }
    unsigned long groupNameLen = space2 - (space1 + 1);
    string groupName = msg.substr(space1 + 1, groupNameLen); //finding the group's name
    if (!regex_match(groupName, validName)) {
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
int main(int argc, char *argv[]) {
    if (argc != 4) {
        cout << "Usage: whasappClient clientName serverAddress serverPort" << endl;
        exit(FAILURE);
    }
    whatsappClient client = whatsappClient(argv[NAME], argv[ADDRESS], argv[PORT]);
    client.runClient();
    exit(SUCCESS);
}