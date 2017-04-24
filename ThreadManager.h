//
// Created by Gilad Lumbroso on 21/03/2017.
//

#ifndef EX2_THREADMANAGER_H
#define EX2_THREADMANAGER_H

#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include "Thread.h"

using namespace std;


class ThreadManager
{
private:
    float quantum;
    int maxThreads;
    void eraseThread(int tid);

public:
    vector<Thread> threads;
    vector<Thread> readyThreads;
    vector<Thread> blockedThreads;

    ThreadManager(int mt);

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

    ~ThreadManager();
};


#endif //EX2_THREADMANAGER_H
