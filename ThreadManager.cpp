//
// Created by Gilad Lumbroso on 21/03/2017.
//

#include <assert.h>
#include "ThreadManager.h"

#define SUCCESS 0
#define MAIN_THREAD 0
#define FAILURE -1
#define SECOND 1000000
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

void setup(Thread &t)
{
    address_t sp, pc;

    sp = (address_t) stack1 + STACK_SIZE - sizeof(address_t);
    pc = (address_t) t.f;
    sigsetjmp(t.env, 1);
    (t.env->__jmpbuf)[JB_SP] = translate_address(sp);
    (t.env->__jmpbuf)[JB_PC] = translate_address(pc);
    sigemptyset(&t.env->__saved_mask);
}

ThreadManager::ThreadManager(int mt)
{
    maxThreads = mt;
    Thread mainThread = Thread(0, nullptr);
    threads.push_back(mainThread);
    readyThreads.push_back(mainThread);
}

int ThreadManager::isThreadExist(int tid)
{
    if (tid >= maxThreads || tid < 0)
    {
        return FAILURE;
    }
    if (threads[tid] == nullptr)
    {
        return FAILURE;
    }
    return SUCCESS;
}

int ThreadManager::addThread(void (*f)(void))
{
    int idx = 0;
    auto it = threads.begin();
    for (it; it != threads.end(); it++)
    {
        if (idx == maxThreads)
        {
            return FAILURE;
        }
        else if (*it == nullptr)
        {
            Thread thread = Thread(idx, f);
            *it = thread;
            readyThreads.push_back(thread);
            setup(thread);
            return idx;
        }
        idx++;
    }
    return FAILURE;
}

int ThreadManager::terminateThread(int tid)
{

    if (tid == 0)
    {
        // TODO: Release all the memory!
        exit(0);
    }

    if (isThreadExist(tid) == FAILURE)
    {
        return FAILURE;
    }

    Thread thread = threads[tid];

    int state = thread.getState();

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

    return SUCCESS;
}

void ThreadManager::eraseThread(int tid)
{
    Thread thread = threads[tid];
    delete thread;
    threads[tid] = nullptr;
}

int ThreadManager::blockThread(int tid)
{
    if (isThreadExist(tid) == FAILURE || tid == MAIN_THREAD)
    {
        return FAILURE;
    }

    Thread thread = threads[tid];

    int state = thread.getState();

    if (state == READY)
    {
        auto pos = find(readyThreads.begin(), readyThreads.end(), thread);
        blockedThreads.push_back(thread);
        readyThreads.erase(pos);
        thread.setState(BLOCKED);
    }
    else if (state == BLOCKED)
    {
        return SUCCESS;
    }

    return SUCCESS;
}

int ThreadManager::resumeThread(int tid, bool isSynced)
{

    if (isThreadExist(tid) == FAILURE)
    {
        return FAILURE;
    }

    Thread thread = threads[tid];

    int state = thread.getState();

    if (state == BLOCKED)
    {
        if (isSynced) {
            thread.isSynced = false;
            if (thread.isBlocked)
            {
                // The thread should stay in the BLOCKED list since it was also blocked manually.
                return SUCCESS;
            }
        }
        else
        {
            thread.isBlocked = false;
            if (thread.isSynced)
            {
                // The thread should stay in the BLOCKED list since it is waiting for a sync.
                return SUCCESS;
            }
        }
        auto pos = find(blockedThreads.begin(), blockedThreads.end(), thread);
        readyThreads.push_back(thread);
        blockedThreads.erase(pos);
        thread.setState(READY);
    }

    return SUCCESS;
}

int ThreadManager::syncThread(int tid)
{

    if (isThreadExist(tid) == FAILURE || tid == MAIN_THREAD)
    {
        return FAILURE;
    }

    Thread thread = threads[tid];
    Thread runningThread = readyThreads[0];

    if (runningThread.getId() == tid)
    {
        return FAILURE;
    }

    // Save the current state of the running thread
    runningThread.saveState();

    // Add the running thread to the sync list of the selected thread.
    thread.sync(runningThread);
    runningThread.isSynced = true;

    // Move the running thread to the blocked list
    readyThreads.erase(readyThreads.begin());
    blockedThreads.push_back(runningThread);

    return SUCCESS;
}

int ThreadManager::runningThreadID()
{
    return readyThreads[0].getId();
}

void ThreadManager::releaseSynced(int tid)
{

    if (isThreadExist(tid) == FAILURE)
    {
        assert("An id of a thread that does not exist was sent to the releaseSynced method.");
    }

    Thread thread = threads[tid];
    vector<Thread> synced = thread.synced;
    vector<Thread> toErase;

    for (auto it = synced.begin(); it != synced.end(); it++)
    {
        Thread t = *it;
        int threadID = t.getId();
        if (isThreadExist(threadID) == FAILURE)
        {
            // The thread was already been terminated.
            toErase.push_back(t);
        }
        else
        {
            resumeThread(t.getId(), true);
        }
    }

    // Erase all the obsolete threads from the synced list.
    for (auto it = toErase.begin(); it != toErase.end(); it++)
    {
        auto pos = find(thread.synced.begin(), thread.synced.end(), *it);
        thread.synced.erase(pos);
    }
}

void ThreadManager::switchThreads(int tid)
{
    if (tid == runningThreadID())
    {
        return;
    }

    Thread t1 = threads[tid];
    Thread runningThread = readyThreads[0];

    int result = runningThread.saveState();
    readyThreads.erase(readyThreads.begin());

    // Move the thread with id tid into the running position
    readyThreads.insert(readyThreads.begin(), t1);
    t1.loadState();
}

int ThreadManager::nextThread()
{
    int nextThreadID = readyThreads[1].getId();
    return nextThreadID;
}

ThreadManager::~ThreadManager()
{

}




