//
// Created by Gilad Lumbroso on 06/20/2017.
//

#include "whatsappServer.h"


#define ERR_MSG "ERROR: "
#define FAILURE 1
#define MAX_PEND 10
#define MSG_SIZE 256

using namespace std;


whatsappServer::whatsappServer(char* port)
{
    //first step
    int portNumber = stoi(port);
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        exitWithError("socket");
    }
    sockaddr_in sa;
    memset(&sa,'\0', sizeof(sockaddr_in));
    sa.sin_family = AF_INET;
    sa.sin_port = htons((uint16_t)portNumber);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    //second step
    int returnVal = bind(serverSocket, (sockaddr*)&sa, sizeof(sa));
    if (returnVal < 0){
        cerr << ERR_MSG << "bind " << errno << endl;
        exit(FAILURE);
    }

    listen(serverSocket, MAX_PEND);
}


void whatsappServer::clearResources() {
    for (auto mapping : userToSocket) {
        close(mapping.second);
    }
    close(serverSocket);
}


void whatsappServer::exitWithError(string msg)
{
    clearResources();
    cerr << ERR_MSG << msg << " " << errno << endl;
    exit(FAILURE);
}


int whatsappServer::connectNewClient()
{
    char username[MSG_SIZE];
    fd_set sockets;

    int newClient = accept(serverSocket, NULL, NULL);
    if (newClient < 0) {
        exitWithError("accept");
    }

    FD_ZERO(&sockets);
    FD_SET(newClient, &sockets);

    timeval timer;
    timer.tv_sec = 1;
    timer.tv_usec = 0;

    int res = select(FD_SETSIZE, &sockets, NULL, NULL, &timer);
    if (res < 0) {
        exitWithError("select");
    } else {
        if (FD_ISSET(newClient, &sockets)) {
            // The socket is set in the sockets set
            ssize_t bytes = read(newClient, username, MSG_SIZE);
            if (bytes < 0) {
                exitWithError("read");
            }
            string msg;
            username[bytes] = '\0';
            if (userToSocket.find(username) == userToSocket.end()) {
                userToSocket[username] = newClient;
                msg = "Connected successfully.\n";
                write(newClient, msg.c_str(), msg.length());
                cout << username << " connected." << endl;
                return newClient;
            } else {
                msg = "Client name is already in use.\n";
                write(newClient, msg.c_str(), msg.length());
                cout << username << " failed to connect." << endl;
                close(newClient);
                return FAILURE;
            }
        }
        exitWithError("FD_ISSET");
    }
    string msg = "Failed to connect to the server";
    write(newClient, msg.c_str(), msg.length());
    cout << "Connection failure" << endl;
    close(newClient);
    return FAILURE;
}

int whatsappServer::runServer()
{
    fd_set sockets;

    while(true) {

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

        int ready = select(MAX_PEND + 1, &sockets, NULL, NULL, &timer);

        if (ready < 0) {
            exitWithError("select");
        } else {
            // Check for ready sockets
            for (auto mapping : userToSocket) {
                int socket = mapping.second;
                if (FD_ISSET(socket, &sockets)) {
                    handleClientRequest();
                }
            }
            if (FD_ISSET(serverSocket, &sockets)) {
                // There is a request for new connection
                connectNewClient();
            } else if (FD_ISSET(STDIN_FILENO, &sockets)) {
                serverInput();
            }
        }
    }
}
