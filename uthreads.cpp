//
// Created by Gilad Lumbroso on 21/03/2017.
//

#include <stdlib.h>
#include <string>
#include <iostream>
#include <signal.h>
#include <sys/time.h>
#include <assert.h>
#include "uthreads.h"
#include "ThreadManager.h"

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
address_t translate_address(address_t addr) {
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

using namespace std;

#define CONVERT 1000000
#define SUCCESS 0
#define FAILURE -1
#define MAIN_THREAD 0
#define INVALID_IDX "Invalid index."
#define JUMP_RETURN_VAL 3


struct sigaction sa;
struct itimerval timer;
int quantum, quantumUsecs, usec, sec;


vector<Thread *> threads;
vector<Thread *> readyThreads;
vector<Thread *> blockedThreads;


// Functions
int isThreadExist(int tid);

void threadSetup(Thread *t);

void releaseSynced(int tid);

int resume(int tid, bool isSynced);

int nextThreadID();


void printThreadError(string message) {
    cerr << "thread library error: " << message << "\n";
}

void printSystemError(string message) {
    cerr << "system error: " << message << "\n";
}

void exitProgram() {
    // TODO: Release memory!
    exit(SUCCESS);
}

void quantum_end(int sig) {

    Thread *running = threads[readyThreads[0]->id];
    Thread *next = threads[nextThreadID()];

    next->quantums++;
    quantum++;

    if (sig == SIGVTALRM) {
        // Move the replaced thread to the back of the ready list.
        readyThreads.erase(readyThreads.begin());
        readyThreads.push_back(running);
    }

    // reset timer
    struct itimerval timer;
    getitimer(ITIMER_VIRTUAL, &timer);
    timer.it_value.tv_sec = sec;
    timer.it_value.tv_usec = usec;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    int ret_val = sigsetjmp(running->env, 1);
    if (ret_val == JUMP_RETURN_VAL) {
        return;
    }

    if (sig == SIGUSR1) {
        // Terminate - delete the current thread and move the next one to the running position.
        readyThreads.erase(readyThreads.begin());
        delete running;
    }

    // Start a virtual timer. It counts down whenever this process is executing.
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL)) {
        cerr << "setitimer error." << '\n';
        return;
    }

    siglongjmp(next->env, JUMP_RETURN_VAL);
}

void init_timer() {
    // Install quantum_end as the signal handler for SIGVTALRM.
    sa.sa_handler = quantum_end;

    usec = quantumUsecs % CONVERT;
    sec = (quantumUsecs - usec) / CONVERT;

    if (sigaction(SIGVTALRM, &sa, NULL) < 0) {
        cerr << "sigaction error." << '\n';
    }

    // Configure the timer to expire after quantum_usecs sec... */
    timer.it_value.tv_sec = sec;        // first time interval, seconds part
    timer.it_value.tv_usec = usec;        // first time interval

    // configure the timer to expire every 3 sec after that.
    timer.it_interval.tv_sec = sec;    // following time intervals, seconds part
    timer.it_interval.tv_usec = usec;    // following time intervals, microseconds part

    // Start a virtual timer. It counts down whenever this process is executing.
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL)) {
        cerr << "setitimer error." << '\n';
    }
}

void threadSetup(Thread *t) {
    address_t sp, pc;
    sp = (address_t) t->stack + STACK_SIZE - sizeof(address_t);
    pc = (address_t) t->f;
    sigsetjmp(t->env, 1);
    (t->env->__jmpbuf)[JB_SP] = translate_address(sp);
    (t->env->__jmpbuf)[JB_PC] = translate_address(pc);
    sigemptyset(&(t->env)->__saved_mask);
}

int uthread_init(int quantum_usecs) {
    if (quantum_usecs <= 0) {
        printThreadError("Quantum should be a positive integer");
        return -1;
    }
    quantumUsecs = quantum_usecs;
    Thread *mainThread = new Thread(0, nullptr);
    threads.push_back(mainThread);
    readyThreads.push_back(mainThread);
    quantum = 1;
    quantumUsecs = quantum_usecs;
    init_timer();
    return SUCCESS;
}

int isThreadExist(int tid) {
    if (tid >= MAX_THREAD_NUM || tid < 0) {
        return FAILURE;
    }
    if (threads[tid] == nullptr) {
        return FAILURE;
    }
    return SUCCESS;
}

int uthread_spawn(void (*f)(void)) {
    int idx = 0;
    auto it = threads.begin();

    sigset_t set, old;
    sigemptyset(&set);
    sigaddset(&set, SIGVTALRM);
    sigprocmask(SIG_SETMASK, &set, &old);

    for (; it != threads.end(); it++) {
        if (idx >= MAX_THREAD_NUM) {
            return FAILURE;
        } else if (*it == nullptr) {
            Thread *thread = new Thread(idx, f);
            threadSetup(thread);
            *it = thread;
            readyThreads.push_back(thread);

            sigprocmask(SIG_SETMASK, &old, NULL);

            return idx;
        }
        idx++;
    }

    Thread *newThread = new Thread(idx, f);
    threadSetup(newThread);
    threads.push_back(newThread);
    readyThreads.push_back(newThread);

    sigprocmask(SIG_SETMASK, &old, NULL);

    return idx;
}

void eraseThread(int tid) {
    Thread *thread = threads[tid];
    delete thread;
    threads[tid] = nullptr;
}

int uthread_terminate(int tid) {
    if (isThreadExist(tid) == FAILURE) {
        return FAILURE;
    }

    // If a thread terminates itself or the main thread is terminated.
    if (tid == MAIN_THREAD) {
        exitProgram();
    }

    Thread *thread = threads[tid];

    int state = thread->getState();
    int nextID = -1;

    sigset_t set, old;
    sigemptyset(&set);
    sigaddset(&set, SIGVTALRM);
    sigprocmask(SIG_SETMASK, &set, &old);

    releaseSynced(tid);

    if (state == READY) {
        auto pos = find(readyThreads.begin(), readyThreads.end(), thread);
        if (pos != readyThreads.end()) {
            if (tid == readyThreads[0]->id) {
                // This is the running thread
                nextID = nextThreadID();
            } else {
                readyThreads.erase(pos);
            }
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

    if (nextID > -1) {
        // The thread that was deleted was itself. Load the next thread env.
        quantum_end(SIGUSR1);
    } else {
        eraseThread(tid);
    }

    sigprocmask(SIG_SETMASK, &old, NULL);

    return SUCCESS;
}

int uthread_block(int tid) {

    if (isThreadExist(tid) == FAILURE || tid == MAIN_THREAD) {
        return FAILURE;
    }

    Thread *thread = threads[tid];

    int state = thread->getState();

    sigset_t set, old;
    sigemptyset(&set);
    sigaddset(&set, SIGVTALRM);
    sigprocmask(SIG_SETMASK, &set, &old);

    if (state == READY) {
        auto pos = find(readyThreads.begin(), readyThreads.end(), thread);
        blockedThreads.push_back(thread);
        readyThreads.erase(pos);
        thread->setState(BLOCKED);
    } else if (state == BLOCKED) {
        return SUCCESS;
    }

    sigprocmask(SIG_SETMASK, &old, NULL);

    return SUCCESS;
}

int resume(int tid, bool isSynced) {
    if (isThreadExist(tid) == FAILURE) {
        return FAILURE;
    }

    Thread *thread = threads[tid];

    int state = thread->getState();

    sigset_t set, old;
    sigemptyset(&set);
    sigaddset(&set, SIGVTALRM);
    sigprocmask(SIG_SETMASK, &set, &old);

    if (state == BLOCKED) {
        if (isSynced) {
            thread->isSynced = false;
            if (thread->isBlocked) {
                // The thread should stay in the BLOCKED list since it was also blocked manually.
                return SUCCESS;
            }
        } else {
            thread->isBlocked = false;
            if (thread->isSynced) {
                // The thread should stay in the BLOCKED list since it is waiting for a sync.
                return SUCCESS;
            }
        }
        auto pos = find(blockedThreads.begin(), blockedThreads.end(), thread);
        readyThreads.push_back(thread);
        blockedThreads.erase(pos);
        thread->setState(READY);
    }

    sigprocmask(SIG_SETMASK, &old, NULL);

    return SUCCESS;
}

int uthread_resume(int tid) {

    int result = resume(tid, false);
    if (result == FAILURE) {
        printThreadError(INVALID_IDX);
        return FAILURE;
    }

    return SUCCESS;
}

int uthread_sync(int tid) {
    if (isThreadExist(tid) == FAILURE || tid == MAIN_THREAD) {
        return FAILURE;
    }

    sigset_t set, old;
    sigemptyset(&set);
    sigaddset(&set, SIGVTALRM);
    sigprocmask(SIG_SETMASK, &set, &old);

    Thread *thread = threads[tid];
    Thread *runningThread = readyThreads[0];

    if (runningThread->id == tid) {
        return FAILURE;
    }

    // Save the current state of the running thread
    int ret_val = sigsetjmp(runningThread->env, 1);
    if (ret_val == JUMP_RETURN_VAL) {
        return SUCCESS;
    }

    // Add the running thread to the sync list of the selected thread.
    thread->sync(runningThread);
    runningThread->isSynced = true;

    // Move the running thread to the blocked list
    readyThreads.erase(readyThreads.begin());
    blockedThreads.push_back(runningThread);

    sigprocmask(SIG_SETMASK, &old, NULL);

    return SUCCESS;
}

void releaseSynced(int tid) {
    if (isThreadExist(tid) == FAILURE) {
        assert("An id of a thread that does not exist was sent to the releaseSynced method.");
    }

    Thread *thread = threads[tid];
    vector<Thread *> synced = thread->synced;
    vector<Thread *> toErase;

    sigset_t set, old;
    sigemptyset(&set);
    sigaddset(&set, SIGVTALRM);
    sigprocmask(SIG_SETMASK, &set, &old);

    for (auto it = synced.begin(); it != synced.end(); it++) {
        Thread *t = *it;
        int threadID = t->id;
        if (isThreadExist(threadID) == FAILURE) {
            // The thread was already been terminated.
            toErase.push_back(t);
        } else {
            resume(t->id, true);
        }
    }

    // Erase all the obsolete threads from the synced list.
    for (auto it = toErase.begin(); it != toErase.end(); it++) {
        auto pos = find(thread->synced.begin(), thread->synced.end(), *it);
        thread->synced.erase(pos);
    }

    sigprocmask(SIG_SETMASK, &old, NULL);
}

int nextThreadID() {
    Thread *thread = readyThreads[1];
    if (thread == nullptr) {
        return readyThreads[0]->id;
    } else {
        int nextThreadID = readyThreads[1]->id;
        return nextThreadID;
    }
}

int uthread_get_tid() {

    sigset_t set, old;
    sigemptyset(&set);
    sigaddset(&set, SIGVTALRM);
    sigprocmask(SIG_SETMASK, &set, &old);
    int id = readyThreads[0]->id;
    sigprocmask(SIG_SETMASK, &old, NULL);
    return id;
}

int uthread_get_total_quantums() {
    return quantum;
}

int uthread_get_quantums(int tid) {

    sigset_t set, old;
    sigemptyset(&set);
    sigaddset(&set, SIGVTALRM);
    sigprocmask(SIG_SETMASK, &set, &old);

    int res = isThreadExist(tid);
    if (res == FAILURE) {
        return FAILURE;
    }
    Thread *thread = threads[tid];
    int quantums = thread->quantums;

    sigprocmask(SIG_SETMASK, &old, NULL);

    return quantums;
}






