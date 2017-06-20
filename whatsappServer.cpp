//
// Created by Gilad Lumbroso on 06/20/2017.
//

#include "whatsappServer.h"


#define ERR_MSG "ERROR: "
#define FAILURE 1
#define MAX_PEND 10


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

void whatsappServer::exitWithError(string msg)
{
    cerr << ERR_MSG << msg << " " << errno << endl;
    exit(FAILURE);
}

int whatsappServer::getConnection()
{
    int t = accept(serverSocket, NULL, NULL);
    if (t < 0) {
        exitWithError("accept");
    }
    return t;
}

int whatsappServer::runServer()
{
    fd_set readfds;
    while(true) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        int ready = select(MAX_PEND, &readfds, NULL, NULL, timer);
    }

}
