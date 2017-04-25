////
//// Created by Gilad Lumbroso on 21/03/2017.
////
//
//#include <assert.h>
//#include <stdlib.h>
//#include "ThreadManager.h"
//
//#define SUCCESS 0
//#define MAIN_THREAD 0
//#define FAILURE -1
//
//
//ThreadManager::ThreadManager(int mt, int quantum_usecs)
//{
//    maxThreads = mt;
//    Thread *mainThread = new Thread(0, nullptr);
//    threads.push_back(mainThread);
//    readyThreads.push_back(mainThread);
//
//    quantum = 1;
//    quantumUsecs = quantum_usecs;
//}
//
//int ThreadManager::isThreadExist(int tid) {
//    if (tid >= maxThreads || tid < 0) {
//        return FAILURE;
//    }
//    if (threads[tid] == nullptr) {
//        return FAILURE;
//    }
//    return SUCCESS;
//}
//
//int ThreadManager::block(sigset_t *newSet, sigset_t *oldSet) {
//    if (sigaddset(newSet, SIGVTALRM) < 0)
//    {
//        return FAILURE;
//    }
//    if (sigprocmask(SIG_SETMASK, newSet, oldSet) < 0)
//    {
//        return FAILURE;
//    }
//    return SUCCESS;
//}
//
//int ThreadManager::unblock(sigset_t *restoreSet) {
//    if (sigprocmask(SIG_SETMASK, restoreSet, NULL) < 0)
//    {
//        return FAILURE;
//    }
//    return SUCCESS;
//}
//
//int ThreadManager::addThread(void (*f)(void))
//{
//    int idx = 0;
//    auto it = threads.begin();
//
//    sigset_t set, old;
//    block(&set, &old);
//
//    for (; it != threads.end(); it++)
//    {
//        if (idx == maxThreads)
//        {
//            return FAILURE;
//        }
//        else if (*it == nullptr)
//        {
//            Thread *thread = new Thread(idx, f);
//            *it = thread;
//            readyThreads.push_back(thread);
//
//            if (unblock(&old) < 0)
//            {
//                return FAILURE;
//            }
//
//            return idx;
//        }
//        idx++;
//    }
//
//    Thread *newThread = new Thread(idx, f);
//    threads.push_back(newThread);
//    readyThreads.push_back(newThread);
//
//    if (unblock(&old) < 0)
//    {
//        return FAILURE;
//    }
//
//    return idx;
//}
//
//int ThreadManager::terminateThread(int tid)
//{
//    if (isThreadExist(tid) == FAILURE)
//    {
//        return FAILURE;
//    }
//
//    Thread *thread = threads[tid];
//
//    int state = thread->getState();
//    int nextThreadID = -1;
//    sigset_t set, old;
//    if (block(&set, &old) < 0)
//    {
//        return FAILURE;
//    }
//
//    releaseSynced(tid);
//
//    if (state == READY)
//    {
//        auto pos = find(readyThreads.begin(), readyThreads.end(), thread);
//        if (pos != readyThreads.end())
//        {
//            if (pos == readyThreads.begin()) {
//                // This is the running thread
//                nextThreadID = nextThread();
//            }
//            readyThreads.erase(pos);
//        }
//        else
//        {
//            assert("There is a problem with the readyThreads.");
//        }
//    }
//    else if (state == BLOCKED)
//    {
//        auto pos = find(blockedThreads.begin(), blockedThreads.end(), thread);
//        if (pos != blockedThreads.end())
//        {
//            blockedThreads.erase(pos);
//        }
//        else
//        {
//            assert("There is a problem with the blockedThreads.");
//        }
//    }
//
//    eraseThread(tid);
//
//    if (nextThreadID > -1)
//    {
//        // The thread that was deleted was itself. Load the next thread env.
//        Thread *running = threads[runningThreadID()];
//        siglongjmp(running->env, 1);
//    }
//
//    if (unblock(&old) < 0)
//    {
//        return FAILURE;
//    }
//
//    return SUCCESS;
//}
//
//void ThreadManager::eraseThread(int tid)
//{
//    Thread *thread = threads[tid];
//    delete thread;
//    threads[tid] = nullptr;
//}
//
//int ThreadManager::blockThread(int tid)
//{
//    if (isThreadExist(tid) == FAILURE || tid == MAIN_THREAD)
//    {
//        return FAILURE;
//    }
//
//    Thread *thread = threads[tid];
//
//    int state = thread->getState();
//
//    sigset_t set, old;
//    if (block(&set, &old) < 0)
//    {
//        return FAILURE;
//    }
//
//    if (state == READY)
//    {
//        auto pos = find(readyThreads.begin(), readyThreads.end(), thread);
//        blockedThreads.push_back(thread);
//        readyThreads.erase(pos);
//        thread->setState(BLOCKED);
//    }
//    else if (state == BLOCKED)
//    {
//        return SUCCESS;
//    }
//
//    if (unblock(&old) < 0)
//    {
//        return FAILURE;
//    }
//
//    return SUCCESS;
//}
//
//int ThreadManager::resumeThread(int tid, bool isSynced)
//{
//    if (isThreadExist(tid) == FAILURE)
//    {
//        return FAILURE;
//    }
//
//    Thread *thread = threads[tid];
//
//    int state = thread->getState();
//
//    sigset_t set, old;
//    if (block(&set, &old) < 0)
//    {
//        return FAILURE;
//    }
//
//    if (state == BLOCKED)
//    {
//        if (isSynced) {
//            thread->isSynced = false;
//            if (thread->isBlocked)
//            {
//                // The thread should stay in the BLOCKED list since it was also blocked manually.
//                return SUCCESS;
//            }
//        }
//        else
//        {
//            thread->isBlocked = false;
//            if (thread->isSynced)
//            {
//                // The thread should stay in the BLOCKED list since it is waiting for a sync.
//                return SUCCESS;
//            }
//        }
//        auto pos = find(blockedThreads.begin(), blockedThreads.end(), thread);
//        readyThreads.push_back(thread);
//        blockedThreads.erase(pos);
//        thread->setState(READY);
//    }
//
//    if (unblock(&old) < 0)
//    {
//        return FAILURE;
//    }
//
//    return SUCCESS;
//}
//
//int ThreadManager::syncThread(int tid)
//{
//    if (isThreadExist(tid) == FAILURE || tid == MAIN_THREAD)
//    {
//        return FAILURE;
//    }
//
//    sigset_t set, old;
//    if (block(&set, &old) < 0)
//    {
//        return FAILURE;
//    }
//
//    Thread *thread = threads[tid];
//    Thread *runningThread = readyThreads[0];
//
//    if (runningThread->getId() == tid)
//    {
//        return FAILURE;
//    }
//
//    // Save the current state of the running thread
//    runningThread->saveState();
//
//    // Add the running thread to the sync list of the selected thread.
//    thread->sync(runningThread);
//    runningThread->isSynced = true;
//
//    // Move the running thread to the blocked list
//    readyThreads.erase(readyThreads.begin());
//    blockedThreads.push_back(runningThread);
//
//    if (unblock(&old) < 0)
//    {
//        return FAILURE;
//    }
//
//    return SUCCESS;
//}
//
//int ThreadManager::runningThreadID()
//{
//    return readyThreads[0]->getId();
//}
//
//void ThreadManager::releaseSynced(int tid)
//{
//
//    if (isThreadExist(tid) == FAILURE)
//    {
//        assert("An id of a thread that does not exist was sent to the releaseSynced method.");
//    }
//
//    Thread *thread = threads[tid];
//    vector<Thread *> synced = thread->synced;
//    vector<Thread *> toErase;
//
//    sigset_t set, old;
//    if (block(&set, &old) < 0)
//
//    for (auto it = synced.begin(); it != synced.end(); it++)
//    {
//        Thread *t = *it;
//        int threadID = t->getId();
//        if (isThreadExist(threadID) == FAILURE)
//        {
//            // The thread was already been terminated.
//            toErase.push_back(t);
//        }
//        else
//        {
//            resumeThread(t->getId(), true);
//        }
//    }
//
//    // Erase all the obsolete threads from the synced list.
//    for (auto it = toErase.begin(); it != toErase.end(); it++)
//    {
//        auto pos = find(thread->synced.begin(), thread->synced.end(), *it);
//        thread->synced.erase(pos);
//    }
//
//    unblock(&old);
//}
//
//void ThreadManager::switchThreads(Thread *running, Thread *next)
//{
//    if (running == nullptr || next == nullptr)
//    {
//        assert("A null pointer was sent to swtichThreads function.");
//    }
//    if (running->getId() == next->getId())
//    {
//        return;
//    }
//
//    int ret_val = sigsetjmp(running->env, 1);
//    if (ret_val == 2)
//    {
//        return;
//    }
//    siglongjmp(next->env, 2);
//}
//
//int ThreadManager::nextThread()
//{
//    Thread *thread = readyThreads[1];
//    if (thread == nullptr)
//    {
//        return runningThreadID();
//    }
//    else
//    {
//        int nextThreadID = readyThreads[1]->getId();
//        return nextThreadID;
//    }
//}
//
//int ThreadManager::getTotalQuantum()
//{
//    return quantum;
//}
//
//ThreadManager::~ThreadManager()
//{
//
//}
//
//
//
//
