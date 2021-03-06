//
// Created by Gilad Lumbroso on 06/20/2017.
//

#include <stdlib.h>
#include "whatsappServer.h"


#define ERR_MSG "ERROR: "
#define FAILURE 1
#define SUCCESS 0
#define MAX_PEND 10
#define MSG_SIZE 256
#define PORT 1
#define ERROR -1
#define PRIME ": "
#define STR_END ".\n"


using namespace std;

/**
 * the main constructor of the whatsappServer
 * @param port - the port we use.
 * @return
 */
whatsappServer::whatsappServer(char *port) {
    //first step
    int portNumber = stoi(port);
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        exitWithError("socket", 0, false);
    }
    sockaddr_in sa;
    memset(&sa, '\0', sizeof(sockaddr_in));
    sa.sin_family = AF_INET;
    sa.sin_port = htons((uint16_t) portNumber);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    //second step
    int res = bind(serverSocket, (sockaddr *) &sa, sizeof(sa));
    if (res < 0) {
        exitWithError("bind", 0, false);
    }

    res = listen(serverSocket, MAX_PEND);
    if (res < 0) {
        exitWithError("listen", 0, false);
    }
}

/**
 * clears all the resources.
 */
void whatsappServer::clearResources() {
    for (auto mapping : userToSocket) {
        close(mapping.second);
    }
    close(serverSocket);
}

/**
 * function that help us to exit the connection well.
 * @param msg - the message we recieve to print to cerr.
 * @param fd - the fd of the socket.
 * @param toClose - if there is need to close the socket of the client.
 */
void whatsappServer::exitWithError(string msg, int fd, bool toClose) {
    clearResources();
    cerr << ERR_MSG << msg << " " << errno << endl;
    if (toClose) {
        close(fd);
    }
    exit(FAILURE);
}

/**
 * try to open new connection.
 */
void whatsappServer::connection() {
    char message[MSG_SIZE];
    int newConnection;
    fd_set fds;
    int returnVal = initConnection(newConnection, fds);
    if (returnVal == ERROR) {
        exitWithError("select", newConnection, true);
        exit(FAILURE);
    }
    if (FD_ISSET(newConnection, &fds)) {
        tryToConnect(message, newConnection);
        return;
    }
    string msgToClient = "Failed to connect the server\n";
    write(newConnection, msgToClient.c_str(), msgToClient.length());
    string strToPrint = (string) message;
    printSituation(strToPrint, " failed to connect.");
    close(newConnection);
}


/**
 * checks if the new socket is already in the set, if not - add.
 * @param message - the message we got for connection.
 * @param newConnection - the fd of the new connection
 */
void whatsappServer::tryToConnect(char *message, int newConnection) {
    ssize_t bytes_read = read(newConnection, message, MSG_SIZE);
    message[bytes_read] = '\0';
    if (userToSocket.find(message) == userToSocket.end()) {
        userToSocket[message] = newConnection;
        write(newConnection, CONNECT_SUCCESS, 24);
        cout << message << " connected." << endl;
    } else {
        string msgToClient = "Client name is already in use.\n";
        write(newConnection, msgToClient.c_str(), msgToClient.length());
        string strToPrint = (string) message;
        printSituation(strToPrint, " failed to connect.");
        close(newConnection);
    }
}

/**
 * init new connection of the server.
 * @param newConnection - the new connection
 * @param fds - fd of the socket.
 * @return
 */
int whatsappServer::initConnection(int &newConnection, fd_set &fds) const {
    newConnection = accept(serverSocket, NULL, NULL);
    FD_ZERO(&fds);
    FD_SET(newConnection, &fds);
    timeval timer;
    timer.tv_sec = 2;
    timer.tv_usec = 0;
    return select(FD_SETSIZE, &fds, NULL, NULL, &timer);
}

/**
 * loop of the server. try to recieve and send messages and treat them.
 * @return
 */
void whatsappServer::runServer() {

    fd_set sockets;

    while (true) {
        FD_ZERO(&sockets);
        FD_SET(serverSocket, &sockets);
        FD_SET(STDIN_FILENO, &sockets);
        for (auto mapping : userToSocket) {
            int socket = mapping.second;
            FD_SET(socket, &sockets);
        }
        timeval timer;
        timer.tv_sec = 1;
        timer.tv_usec = 0;

        int ready = select(FD_SETSIZE, &sockets, NULL, NULL, &timer);
        if (ready < 0) {
            for (auto mapping : userToSocket) {
                close(mapping.second);
            }
            exitWithError("select", 0, false);
        }
        if (FD_ISSET(STDIN_FILENO, &sockets)) {
            string input;
            getline(cin, input);
            transform(input.begin(), input.end(), input.begin(), ::tolower);
            if (input.compare("exit") == 0) {
                for (auto pair : userToSocket) {
                    close(pair.second);
                }
                close(serverSocket);
                exit(SUCCESS);
            }
        }
        if (FD_ISSET(serverSocket, &sockets)) {
            connection();
        }
        for (auto pair : userToSocket) {
            if (FD_ISSET(pair.second, &sockets)) {
                commandParser(pair.first, pair.second);
            }
        }
    }
}

/**
 * print the message and the information on it.
 */
void whatsappServer::printSituation(string msg, string msg2) {
    cout << msg << msg2 << endl;
}

/**
 * parser of the command we got from the client.
 * @param userName - the client who wrote the command.
 * @param fd - the file descriptor of the socket.
 */
void whatsappServer::commandParser(string userName, int fd) {
    char message[MSG_SIZE];
    ssize_t bytes_read = read(fd, message, 256);
    message[bytes_read] = '\0';

    string msg = (string) message;
    if (msg.find(WHO) == 0) {
        checkConnections(userName, fd);
    } else if (msg.find(SEND) == 0) {
        sendMessage(userName, fd, msg);
    } else if (msg.find(CREATE_GROUP) == 0) {
        group(userName, fd, msg);
    } else if (msg.find("exit") == 0) {
        clientExit(userName, fd);
    }
}

/**
 * checks the connections we have. in case the client wrote "who"
 * @param user - the user who want to check.
 * @param fd - the fd of the socket.
 */
void whatsappServer::checkConnections(string user, int fd) {
    vector<string> connected;
    // if not empty
    if (userToSocket.size() != 0) {
        for (auto it = userToSocket.begin(); it != userToSocket.end(); ++it) {
            connected.push_back(it->first);
        }
        sort(connected.begin(), connected.end());
        string res = connected[0];
        for (unsigned int i = 1; i < connected.size(); ++i) {
            res += "," + connected[i];
        }
        res += STR_END;
        write(fd, res.c_str(), res.length());
        printSituation(user, ": Requests the currently connected client names.");
    } else {
        string msg = "";
        write(fd, msg.c_str(), msg.length());
        printSituation(user, ": Requests the currently connected client names.");
    }
}

/**
 * in case the client send message to exit from the whatsappServer
 * @param user - the user who wants to disconnect.
 * @param fd - file descriptor of the socket.
 */
void whatsappServer::clientExit(string user, int fd) {
    //check if the user in groups.
    for (auto it = groupUserNames.begin(); it != groupUserNames.end(); ++it) {
        vector<string> groupUsers = it->second;
        vector<string>::iterator thisUserIter = find(groupUsers.begin(), groupUsers.end(), user);
        // is user in the group?
        if (thisUserIter != groupUsers.end()) {
            groupUsers.erase(thisUserIter);
        }
    }
    userToSocket.erase(user);
    string msg = "Unregistered successfully.\n";
    write(fd, msg.c_str(), msg.length());
    close(fd);
    printSituation(user, ": Unregistered successfully.");
}

/**
 * when we decided that group is the current operation
 * @param user - the user who want to do that operation
 * @param fd - the fd of the socket.
 * @param msg - the message that the server got.
 */
void whatsappServer::group(string user, int fd, string msg) {
    size_t space1 = msg.find(" ", 0);
    size_t space2 = msg.find(" ", space1 + 1);
    size_t stringLength = space2 - (space1 + 1);
    string nameOfGroup = msg.substr(space1 + 1, stringLength);

    if (!(userToSocket.find(nameOfGroup) != userToSocket.end() ||
          groupUserNames.find(nameOfGroup) != groupUserNames.end())) {
        createGroup(user, fd, msg, space2, nameOfGroup);
    } else {
        // can not create group
        string msgToClient = "ERROR: failed to create group \"" + nameOfGroup + "\".";
        printSituation(user, PRIME + msgToClient);
        msgToClient = msgToClient + "\n";
        write(fd, msgToClient.c_str(), msgToClient.length());
    }
}

/**
 * in case a user want to create group.
 * @param user - the user who want to create the group.
 * @param fd - the fd of the socket.
 * @param msg - the msg we got from the user.
 * @param space2 - the second space in the msg.
 * @param nameOfGroup - the name of the group we want to open.
 */
void whatsappServer::createGroup(string &user, int fd, const string &msg, size_t space2,
                                 const string &nameOfGroup) {
    vector<string> clientsNames;
    string clients = msg.substr(space2 + 1);
    size_t curComma = clients.find(",");
    //adding the clients to the vector
    while (curComma != string::npos) {
        //list of separated by commas
        string client = clients.substr(0, curComma);
        if ((find(clientsNames.begin(), clientsNames.end(), client)) == clientsNames.end()) {
            if (userToSocket.find(client) != userToSocket.end()) {
                clientsNames.push_back(client);
            }
        }
        clients = clients.substr(curComma + 1);
        curComma = clients.find(",");
    }
    //adding the last client, not separated by comma
    clients.pop_back();
    if ((find(clientsNames.begin(), clientsNames.end(), clients)) == clientsNames.end()) {
        if (userToSocket.find(clients) != userToSocket.end()) {
            clientsNames.push_back(clients);
        }
    }
    //adding the user that created the group
    if ((find(clientsNames.begin(), clientsNames.end(), user)) == clientsNames.end()) {
        clientsNames.push_back(user);
    }
    // case of single member
    if (clientsNames.size() < 2) {
        string prime = ": ";
        string msgToClient = "ERROR: failed to create group \"" + nameOfGroup + "\".";
        printSituation(user, prime + msgToClient);
        msgToClient = msgToClient + "\n";
        write(fd, msgToClient.c_str(), msgToClient.length());
    } else {
        //adding group and clients to our map.
        groupUserNames[nameOfGroup] = clientsNames;
        string msgToClient = "Group \"" + nameOfGroup + "\" was created successfully.";
        printSituation(user, PRIME + msgToClient);
        write(fd, msgToClient.c_str(), msgToClient.length());
    }
}

/**
 * if we decided that the server should send a message
 * @param user - the user who sent the message
 * @param fd - the fd of the socket
 * @param msg - the message
 */
void whatsappServer::sendMessage(string user, int fd, string msg) {

    size_t space1 = msg.find(" ", 0);
    size_t space2 = msg.find(" ", space1 + 1);
    size_t nameLength = (space2 - (space1 + 1));

    string name = msg.substr(space1 + 1, nameLength);
    string clientsMsg = msg.substr(space2 + 1);
    clientsMsg.pop_back();
    if (user == name) { //the user tried to send himself a msg
        selfSend(user, fd, name, clientsMsg);
        return;
    }
    string msgToSend = user + ": " + clientsMsg + "\n";
    //checks if its a user name
    if ((userToSocket.find(name) != userToSocket.end()) && (name != user)) {
        int nameFd = userToSocket[name];
        write(nameFd, msgToSend.c_str(), msgToSend.length());
        cout << user << ": \"" << clientsMsg << "\" was sent successfully to " << name << "." << endl;
        string msgTo = "Sent successfully.\n";
        write(fd, msgTo.c_str(), msgTo.length());

    }
        //checks if its a group with the given name
    else if (groupUserNames.find(name) != groupUserNames.end()) {
        vector<string> groupMembers = groupUserNames[name];
        //checks if the user is in this group
        if (find(groupMembers.begin(), groupMembers.end(), user) != groupMembers.end()) {
            for (unsigned int i = 0; i < groupMembers.size(); i++) {
                int nameFd = userToSocket[groupMembers[i]];
                if (nameFd != fd) {
                    write(nameFd, msgToSend.c_str(), msgToSend.length());
                }
            }
            string msg3 = ": \"" + clientsMsg + "\" was sent successfully to " + name + ".";
            printSituation(user, msg3);
            string msg4 = "Sent successfully.\n";
            write(fd, msg4.c_str(), msg4.length());
        } else { //the user isn't in this group
            string msg3 = "ERROR: failed to send";
            string toCout = PRIME + msg3 + " \"" + clientsMsg + "\" to " + name + ".";
            printSituation(user, toCout);
            string toWrite = msg3 + STR_END;
            write(fd, toWrite.c_str(), toWrite.length());
        }
    }
        // there is no group or user with the name.
    else {
        string base = "ERROR: failed to send";
        string toCout = PRIME + base + " \"" + clientsMsg + "\" to " + name + ".";
        printSituation(user, toCout);
        string toWrite = base + STR_END;
        write(fd, toWrite.c_str(), toWrite.length());
    }
}

/**
 * in case we send a message to ourselve.
 * @param user - the user who send the message
 * @param fd - file descriptor of the socket.
 * @param name - the name of the user we want to send to.
 * @param clientsMsg - the msg.
 */
void whatsappServer::selfSend(string &user, int fd, const string &name, const string &clientsMsg) {
    string msg2 = "ERROR: failed to send";
    string end = " \"" + clientsMsg + "\" to " + name + ".";
    printSituation(user, PRIME + msg2 + end);
    string toWrite = msg2 + STR_END;
    write(fd, toWrite.c_str(), toWrite.length());
}

/**
 * the main function of the whatsappServer.
 * @param argc - num of inputs.
 * @param argv - the pointer to the string input.
 * @return
 */
int main(int argc, char **argv) {
    if (argc != 2) {
        cout << "Usage: whatsappServer portNum" << endl;
        exit(FAILURE);
    }
    whatsappServer s = whatsappServer(argv[PORT]);
    s.runServer();
    exit(SUCCESS);
}