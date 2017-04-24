//
// Created by Gilad Lumbroso on 21/03/2017.
//

#ifndef EX2_THREADMANAGER_H
#define EX2_THREADMANAGER_H

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <setjmp.h>
#include <unistd.h>
#include "Thread.h"

using namespace std;


class ThreadManager
{
private:
    int maxThreads;
    void eraseThread(int tid);
    int quantumUsecs;

    void (*handler)(int);

    int block(sigset_t *newSet, sigset_t *oldSet);
    int unblock(sigset_t *restoreSet);

public:
    vector<Thread*> threads;
    vector<Thread*> readyThreads;
    vector<Thread*> blockedThreads;

    int quantum;

    ThreadManager(int mt, int quantum_usecs);

    int isThreadExist(int tid);

    int addThread(void (*f)(void));

    int terminateThread(int tid);

    int blockThread(int tid);

    int resumeThread(int tid, bool isSynced);

    int syncThread(int tid);

    int runningThreadID();

    void releaseSynced(int tid);

    void switchThreads(int tid);

    int nextThread();

    int getTotalQuantum();

    ~ThreadManager();
};


#endif //EX2_THREADMANAGER_H
