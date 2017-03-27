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
public:
    ThreadManager(int mt);
    int addThread(void (*f)(void));
    int terminateThread(int tid);
    ~ThreadManager();
};


#endif //EX2_THREADMANAGER_H
