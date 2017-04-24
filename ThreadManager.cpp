//
// Created by Gilad Lumbroso on 21/03/2017.
//

#include <assert.h>
#include <stdlib.h>
#include "ThreadManager.h"

#define SUCCESS 0
#define MAIN_THREAD 0
#define FAILURE -1
#define STACK_SIZE 4096

char stack1[STACK_SIZE];
char stack2[STACK_SIZE];

#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
            "rol    $0x11,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
        "rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#endif

void setup(Thread *thread)
{
    address_t sp, pc;
    Thread t = *thread;
    sp = (address_t) stack1 + STACK_SIZE - sizeof(address_t);
    pc = (address_t) t.f;
    sigsetjmp(t.env, 1);
    (t.env->__jmpbuf)[JB_SP] = translate_address(sp);
    (t.env->__jmpbuf)[JB_PC] = translate_address(pc);
    sigemptyset(&t.env->__saved_mask);
}

ThreadManager::ThreadManager(int mt, int quantum_usecs)
{
    maxThreads = mt;
    Thread *mainThread = new Thread(0, nullptr);
    threads.push_back(mainThread);
    readyThreads.push_back(mainThread);

    quantum = 1;
    quantumUsecs = quantum_usecs;
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

int ThreadManager::block(sigset_t *newSet, sigset_t *oldSet) {
    if (sigaddset(newSet, SIGVTALRM) < 0)
    {
        return FAILURE;
    }
    if (sigprocmask(SIG_SETMASK, newSet, oldSet) < 0)
    {
        return FAILURE;
    }
    return 0;
}

int ThreadManager::unblock(sigset_t *restoreSet) {
    if (sigprocmask(SIG_SETMASK, restoreSet, NULL) < 0)
    {
        return FAILURE;
    }
    return 0;
}

int ThreadManager::addThread(void (*f)(void))
{
    int idx = 0;
    auto it = threads.begin();

    sigset_t set, old;
    block(&set, &old);

    for (; it != threads.end(); it++)
    {
        if (idx == maxThreads)
        {
            return FAILURE;
        }
        else if (*it == nullptr)
        {
            Thread *thread = new Thread(idx, f);
            *it = thread;
            readyThreads.push_back(thread);
            setup(thread);

            if (unblock(&old) < 0)
            {
                return FAILURE;
            }

            return idx;
        }
        idx++;
    }

    Thread *newThread = new Thread(idx, f);
    threads.push_back(newThread);
    readyThreads.push_back(newThread);
    setup(newThread);

    if (unblock(&old) < 0)
    {
        return FAILURE;
    }

    return idx;
}

int ThreadManager::terminateThread(int tid)
{
    if (isThreadExist(tid) == FAILURE)
    {
        return FAILURE;
    }

    Thread *thread = threads[tid];

    int state = thread->getState();

    sigset_t set, old;
    if (block(&set, &old) < 0)
    {
        return FAILURE;
    }

    releaseSynced(tid);

    if (state == READY)
    {
        auto pos = find(readyThreads.begin(), readyThreads.end(), thread);
        if (pos != readyThreads.end())
        {
            if (pos == readyThreads.begin()) {
                // This is the running thread
                int nextThreadID = nextThread();
                switchThreads(nextThreadID);
            }
            readyThreads.erase(pos);
        }
        else
        {
            assert("There is a problem with the readyThreads.");
        }
    }
    else if (state == BLOCKED)
    {
        auto pos = find(blockedThreads.begin(), blockedThreads.end(), thread);
        if (pos != blockedThreads.end())
        {
            blockedThreads.erase(pos);
        }
        else
        {
            assert("There is a problem with the blockedThreads.");
        }
    }

    eraseThread(tid);

    if (unblock(&old) < 0)
    {
        return FAILURE;
    }

    return SUCCESS;
}

void ThreadManager::eraseThread(int tid)
{
    Thread *thread = threads[tid];
    delete thread;
    threads[tid] = nullptr;
}

int ThreadManager::blockThread(int tid)
{
    if (isThreadExist(tid) == FAILURE || tid == MAIN_THREAD)
    {
        return FAILURE;
    }

    Thread *thread = threads[tid];

    int state = thread->getState();

    sigset_t set, old;
    if (block(&set, &old) < 0)
    {
        return FAILURE;
    }

    if (state == READY)
    {
        auto pos = find(readyThreads.begin(), readyThreads.end(), thread);
        blockedThreads.push_back(thread);
        readyThreads.erase(pos);
        thread->setState(BLOCKED);
    }
    else if (state == BLOCKED)
    {
        return SUCCESS;
    }

    if (unblock(&old) < 0)
    {
        return FAILURE;
    }

    return SUCCESS;
}

int ThreadManager::resumeThread(int tid, bool isSynced)
{
    if (isThreadExist(tid) == FAILURE)
    {
        return FAILURE;
    }

    Thread *thread = threads[tid];

    int state = thread->getState();

    sigset_t set, old;
    if (block(&set, &old) < 0)
    {
        return FAILURE;
    }

    if (state == BLOCKED)
    {
        if (isSynced) {
            thread->isSynced = false;
            if (thread->isBlocked)
            {
                // The thread should stay in the BLOCKED list since it was also blocked manually.
                return SUCCESS;
            }
        }
        else
        {
            thread->isBlocked = false;
            if (thread->isSynced)
            {
                // The thread should stay in the BLOCKED list since it is waiting for a sync.
                return SUCCESS;
            }
        }
        auto pos = find(blockedThreads.begin(), blockedThreads.end(), thread);
        readyThreads.push_back(thread);
        blockedThreads.erase(pos);
        thread->setState(READY);
    }

    if (unblock(&old) < 0)
    {
        return FAILURE;
    }

    return SUCCESS;
}

int ThreadManager::syncThread(int tid)
{
    if (isThreadExist(tid) == FAILURE || tid == MAIN_THREAD)
    {
        return FAILURE;
    }

    sigset_t set, old;
    if (block(&set, &old) < 0)
    {
        return FAILURE;
    }

    Thread *thread = threads[tid];
    Thread *runningThread = readyThreads[0];

    if (runningThread->getId() == tid)
    {
        return FAILURE;
    }

    // Save the current state of the running thread
    runningThread->saveState();

    // Add the running thread to the sync list of the selected thread.
    thread->sync(runningThread);
    runningThread->isSynced = true;

    // Move the running thread to the blocked list
    readyThreads.erase(readyThreads.begin());
    blockedThreads.push_back(runningThread);

    if (unblock(&old) < 0)
    {
        return FAILURE;
    }

    return SUCCESS;
}

int ThreadManager::runningThreadID()
{
    return readyThreads[0]->getId();
}

void ThreadManager::releaseSynced(int tid)
{

    if (isThreadExist(tid) == FAILURE)
    {
        assert("An id of a thread that does not exist was sent to the releaseSynced method.");
    }

    Thread *thread = threads[tid];
    vector<Thread *> synced = thread->synced;
    vector<Thread *> toErase;

    sigset_t set, old;
    if (block(&set, &old) < 0)

    for (auto it = synced.begin(); it != synced.end(); it++)
    {
        Thread *t = *it;
        int threadID = t->getId();
        if (isThreadExist(threadID) == FAILURE)
        {
            // The thread was already been terminated.
            toErase.push_back(t);
        }
        else
        {
            resumeThread(t->getId(), true);
        }
    }

    // Erase all the obsolete threads from the synced list.
    for (auto it = toErase.begin(); it != toErase.end(); it++)
    {
        auto pos = find(thread->synced.begin(), thread->synced.end(), *it);
        thread->synced.erase(pos);
    }

    unblock(&old);
}

void ThreadManager::switchThreads(int tid)
{
    Thread *t1 = threads[tid];
    Thread *runningThread = readyThreads[0];

    t1->quantums++;
    quantum++;

    if (tid == runningThreadID())
    {
        return;
    }

    sigset_t set, old;
    block(&set, &old);

    runningThread->saveState();
    readyThreads.erase(readyThreads.begin());
    t1->loadState();

    unblock(&old);
}

int ThreadManager::nextThread()
{
    Thread *thread = readyThreads[1];
    if (thread == nullptr)
    {
        return runningThreadID();
    }
    else
    {
        int nextThreadID = readyThreads[1]->getId();
        return nextThreadID;
    }
}

int ThreadManager::getTotalQuantum()
{
    return quantum;
}

ThreadManager::~ThreadManager()
{

}




