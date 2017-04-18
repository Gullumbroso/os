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
    void switchThreads(int tid1);

public:
    vector<Thread> threads;
    vector<Thread> readyThreads;
    vector<Thread> blockedThreads;
    int syncThreadsCounter;

    ThreadManager(int mt);

    int isThreadExist(int tid);

    int addThread(void (*f)(void));

    int terminateThread(int tid);

    int blockThread(int tid);

    int resumeThread(int tid);

    int syncThread(int tid);

    int runningThreadID();

    ~ThreadManager();
};


#endif //EX2_THREADMANAGER_H
