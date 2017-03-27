//
// Created by Gilad Lumbroso on 21/03/2017.
//

#include <assert.h>
#include "ThreadManager.h"

ThreadManager::ThreadManager(int mt)
{
    maxThreads = mt;
    Thread mainThread = Thread(0, nullptr);
    threads.push_back(mainThread);
    readyThreads.push_back(mainThread);
}

int ThreadManager::addThread(void (*f)(void))
{
    int idx = 0;
    auto it = threads.begin();
    for (it; it != threads.end(); it++){
        if (idx == maxThreads) {
            return -1;
        }
        else if (*it == nullptr) {
            Thread thread = Thread(idx, f);
            *it = thread;
            readyThreads.push_back(thread);
            return idx;
        }
        idx++;
    }
    return -1;
}

int ThreadManager::terminateThread(int tid)
{
    if (tid >= maxThreads || tid < 0) {
        return -1;
    }
    Thread thread = threads[tid];
    int state = thread.getState();
    if (state == READY) {
        auto pos = find(readyThreads.begin(), readyThreads.end(), thread);
        if (pos != readyThreads.end()) {
            readyThreads.erase(pos);
        } else {
            assert("There is a problem with the readyThreads.");
        }
    } else if (state == BLOCKED) {
        auto pos = find(blockedThreads.begin(), blockedThreads.end(), thread);
        if (pos != blockedThreads.end()) {
            blockedThreads.erase(pos);
        } else {
            assert("There is a problem with the blockedThreads.");
        }
    }
    threads[tid] = nullptr;

    // Handle the cases of terminating the main thread and a thread terminating itself.

    return 0;
}

ThreadManager::~ThreadManager()
{

}