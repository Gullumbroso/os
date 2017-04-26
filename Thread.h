//
// Created by Gilad Lumbroso on 21/03/2017.
//

#ifndef EX2_THREAD_H
#define EX2_THREAD_H

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <vector>
#include <ostream>
#include <stdlib.h>
#include <algorithm>

using namespace std;

#define STACK_SIZE 4096
#define READY 0
#define BLOCKED 2


class Thread
{
private:
    int state;

public:
    Thread(int id, void (*f)(void));
    ~Thread();

    int id;
    void (*f)(void);
    sigjmp_buf env;
    char stack[STACK_SIZE];

    vector<Thread *> synced;

    int getState();
    int quantums;
    void setState(int s);
    void sync(Thread *t);
    bool isSynced;
    bool isBlocked;

    bool operator==(const Thread &other) const;
    bool operator!=(const Thread &other) const;
};


#endif //EX2_THREAD_H
