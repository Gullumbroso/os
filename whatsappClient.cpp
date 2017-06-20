//
// Created by Gilad Lumbroso on 06/20/2017.
//

#include "whatsappClient.h"


void whatsappClient::exitWithError(string msg)
{
    cerr << ERR_MSG << msg << " " << errno << endl;
    exit(FAILURE);
}


int whatsappClient::callSocket(char *hostname, unsigned short portnum)
{
    struct sockaddr_in sa;
    struct hostent *hp;
    int s;
    if ((hp = gethostbyname(hostname)) == NULL) {
        return(-1);
    }
    memset(&sa,0,sizeof(sa));
    memcpy((char *)&sa.sin_addr, hp->h_addr, hp->h_length);
    sa.sin_family= hp->h_addrtype;
    sa.sin_port= htons((u_short)portnum);
    s = socket(hp->h_addrtype, SOCK_STREAM,0);
    if (s < 0) {
        exitWithError("socket");
    }
    int res = connect(s, (struct sockaddr*)&sa, sizeof(sa));
    if (res < 0) {
        close(s);
        exitWithError("connect");
    }

    return s;
}


int whatsappClient::readData(int s, char *buf, int n)
{
    int bcount = 0;
    int br = 0;

    while (bcount < n) {
        br = (int) read(s, buf, (unsigned int)(n - bcount));
        if (br > 0) {
            bcount += br;
            buf += br;
        }
        if (br < 1) {
            exitWithError("read");
        }
    }

    return bcount;
}
