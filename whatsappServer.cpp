//
// Created by Gilad Lumbroso on 06/20/2017.
//

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


void whatsappServer::clearResources() {
    for (auto mapping : userToSocket) {
        close(mapping.second);
    }
    close(serverSocket);
}


void whatsappServer::exitWithError(string msg, int fd, bool toClose) {
    clearResources();
    cerr << ERR_MSG << msg << " " << errno << endl;
    if (toClose) {
        close(fd);
    }
    exit(FAILURE);
}

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

int whatsappServer::initConnection(int &newConnection, fd_set &fds) const {
    newConnection = accept(serverSocket, NULL, NULL);
    FD_ZERO(&fds);
    FD_SET(newConnection, &fds);
    timeval timer;
    timer.tv_sec = 2;
    timer.tv_usec = 0;
    return select(FD_SETSIZE, &fds, NULL, NULL, &timer);
}

int whatsappServer::runServer() {

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

void whatsappServer::printSituation(string msg, string msg2) {
    cout << msg << msg2 << endl;
}

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

void whatsappServer::clientExit(string user, int fd) {
    //check if the user in groups.
    for (auto it = groupUsernames.begin(); it != groupUsernames.end(); ++it) {
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

void whatsappServer::group(string user, int fd, string msg) {
    size_t space1 = msg.find(" ", 0);
    size_t space2 = msg.find(" ", space1 + 1);
    size_t stringLength = space2 - (space1 + 1);
    string nameOfGroup = msg.substr(space1 + 1, stringLength);

    if (!(userToSocket.find(nameOfGroup) != userToSocket.end() ||
          groupUsernames.find(nameOfGroup) != groupUsernames.end())) {
        createGroup(user, fd, msg, space2, nameOfGroup);
    } else {
        // can not create group
        string msgToClient = "ERROR: failed to create group \"" + nameOfGroup + "\".";
        printSituation(user, PRIME + msgToClient);
        msgToClient = msgToClient + "\n";
        write(fd, msgToClient.c_str(), msgToClient.length());
    }
}

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
        groupUsernames[nameOfGroup] = clientsNames;
        string msgToClient = "Group \"" + nameOfGroup + "\" was created successfully.";
        printSituation(user, PRIME + msgToClient);
        write(fd, msgToClient.c_str(), msgToClient.length());
    }
}

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
        cout << user << ": \"" << clientsMsg << "\" was sent successfully to " << name << "."
             << endl;
        string msgTo = "Sent successfully.\n";
        write(fd, msgTo.c_str(), msgTo.length() + 1);

    }
        //checks if its a group with the given name
    else if (groupUsernames.find(name) != groupUsernames.end()) {
        vector<string> groupMembers = groupUsernames[name];
        //checks if the user is in this group
        if (find(groupMembers.begin(), groupMembers.end(), user) != groupMembers.end()) {
            for (unsigned int i = 0; i < groupMembers.size(); i++) {
                int nameFd = userToSocket[groupMembers[i]];
                if (nameFd != fd) {
                    write(nameFd, msgToSend.c_str(), msgToSend.length() + 1);
                }
            }
            string msg3 = ": \"" + clientsMsg + "\" was sent successfully to " + name + ".";
            printSituation(user, msg3);
            msg3 = "Sent successfully.\n";
            write(fd, msg3.c_str(), msg3.length());
        } else { //the user isn't in this group
            string msg3 = "ERROR: failed to send";
            string toCout = PRIME + msg3 + " \"" + clientsMsg + "\" to " + name + ".";
            printSituation(user, toCout);
            string toWrite = msg3 + STR_END;
            write(fd, toWrite.c_str(), toWrite.length() + 1);
        }
    }
        // there is no group or user with the name.
    else {
        string base = "ERROR: failed to send";
        string toCout = PRIME + base + " \"" + clientsMsg + "\" to " + name + ".";
        printSituation(user, toCout);
        string toWrite = base + STR_END;
        write(fd, toWrite.c_str(), toWrite.length() + 1);
    }
}

void whatsappServer::selfSend(string &user, int fd, const string &name, const string &clientsMsg) {
    string msg2 = "ERROR: failed to send";
    string end = " \"" + clientsMsg + "\" to " + name + ".";
    printSituation(user, PRIME + msg2 + end);
    string toWrite = msg2 + STR_END;
    write(fd, toWrite.c_str(), toWrite.length() + 1);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        cout << "Usage: whatsappServer portNum" << endl;
        exit(FAILURE);
    }
    whatsappServer s = whatsappServer(argv[PORT]);
    s.runServer();
    exit(SUCCESS);
}