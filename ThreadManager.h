//
// Created by Gilad Lumbroso on 21/03/2017.
//

#ifndef EX2_THREADMANAGER_H
#define EX2_THREADMANAGER_H

#include <stdlib.h>
#include <vector>
#include <algorithm>
#include "Thread.h"

using namespace std;


class ThreadManager
{
private:
    vector<Thread> threads;
    vector<Thread> readyThreads;
    vector<Thread> blockedThreads;
    float quantum;
    int maxThreads;
    int syncThreadsCounter;
public:
    ThreadManager(int mt);
    int isThreadExist(int tid);
    int addThread(void (*f)(void));
    int terminateThread(int tid);
    int blockThread(int tid);
    ~ThreadManager();

    int resumeThread(int tid);

    int syncThread(int tid);

    int runningThreadID();
};


#endif //EX2_THREADMANAGER_H
