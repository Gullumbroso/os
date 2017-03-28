//
// Created by Gilad Lumbroso on 21/03/2017.
//

#include <assert.h>
#include "ThreadManager.h"

#define SUCCESS 0
#define MAIN_THREAD 0
#define FAILURE -1

ThreadManager::ThreadManager(int mt) {
    syncThreadsCounter = 0;
    maxThreads = mt;
    Thread mainThread = Thread(0, nullptr);
    threads.push_back(mainThread);
    readyThreads.push_back(mainThread);
}

int ThreadManager::isThreadExist(int tid) {
    if (tid >= maxThreads || tid < 0) {
        return FAILURE;
    }
    if (threads[tid] == nullptr) {
        return FAILURE;
    }
    return SUCCESS;
}

int ThreadManager::addThread(void (*f)(void)) {
    int idx = 0;
    auto it = threads.begin();
    for (it; it != threads.end(); it++) {
        if (idx == maxThreads) {
            return FAILURE;
        } else if (*it == nullptr) {
            Thread thread = Thread(idx, f);
            *it = thread;
            readyThreads.push_back(thread);
            return idx;
        }
        idx++;
    }
    return FAILURE;
}

int ThreadManager::terminateThread(int tid) {
    if (isThreadExist(tid) == FAILURE) {
        return FAILURE;
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

    return SUCCESS;
}

int ThreadManager::blockThread(int tid) {

    if (isThreadExist(tid) == FAILURE || tid == MAIN_THREAD) {
        return FAILURE;
    }

    Thread thread = threads[tid];

    int state = thread.getState();

    if (state == READY) {
        auto pos = find(readyThreads.begin(), readyThreads.end(), thread);
        blockedThreads.push_back(thread);
        readyThreads.erase(pos);
        thread.setState(BLOCKED);
    } else if (state == BLOCKED) {
        return SUCCESS;
    }

    return SUCCESS;
}

int ThreadManager::resumeThread(int tid) {

    if (isThreadExist(tid) == FAILURE) {
        return FAILURE;
    }

    Thread thread = threads[tid];
    int state = thread.getState();

    if (state == BLOCKED) {
        auto pos = find(blockedThreads.begin(), blockedThreads.end(), thread);
        readyThreads.push_back(thread);
        blockedThreads.erase(pos);
        thread.setState(READY);
    }

    return SUCCESS;
}

int ThreadManager::syncThread(int tid) {

    if (isThreadExist(tid) == FAILURE || tid == MAIN_THREAD) {
        return FAILURE;
    }

    Thread thread = threads[tid];
    Thread running_thread = readyThreads[0];

    // Move the running thread to the blocked list
    readyThreads.erase(readyThreads.begin());
    blockedThreads.insert(blockedThreads.begin(), running_thread);

    // Increment the sync threads counter
    syncThreadsCounter++;

    // Move the thread with id tid into the running position
    readyThreads.insert(readyThreads.begin(), thread);

    return SUCCESS;
}

int ThreadManager::runningThreadID() {
    return readyThreads[0].getId();
}

int switchThreads()
{

}

ThreadManager::~ThreadManager() {

}




