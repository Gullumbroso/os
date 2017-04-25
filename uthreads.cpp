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

using namespace std;

#define CONVERT 1000000
#define SUCCESS 0
#define FAILURE -1
#define MAIN_THREAD 0
#define INVALID_IDX "Invalid index."


ThreadManager *tm;
struct sigaction sa;
struct itimerval timer;
int quantumUsecs, usec, sec;

void printThreadError(string message)
{
    cerr << "thread library error: "  << message << "\n";
}

void printSystemError(string message)
{
    cerr << "system error: "  << message << "\n";
}

void exitProgram() {
    // TODO: Release memory!
    exit(0);
}

void timer_handler(int sig)
{
    if (sig == SIGVTALRM)
    {
        int replacedThreadID = tm->runningThreadID();
        int nextThreadID = tm->nextThread();

//        // Check if the next therad is not the current one.
//        if (nextThreadID == replacedThreadID)
//        {
//            return;
//        }

        Thread *replacedThread = tm->threads[replacedThreadID];
        Thread *newRunning = tm->threads[nextThreadID];

        // Move the replaced thread to the back of the ready list.
        tm->readyThreads.erase(tm->readyThreads.begin());
        tm->readyThreads.push_back(replacedThread);

        newRunning->quantums++;
        tm->quantum++;

        // reset timer
        struct itimerval timer;
        getitimer(ITIMER_VIRTUAL, &timer);
        timer.it_value.tv_sec = sec;
        timer.it_value.tv_usec = usec;
        timer.it_interval.tv_sec = 0;
        timer.it_interval.tv_usec = 0;

        int ret_val = sigsetjmp(replacedThread->env, 1);
        if (ret_val == 2)
        {
            return;
        }

        // Start a virtual timer. It counts down whenever this process is executing.
        if (setitimer (ITIMER_VIRTUAL, &timer, NULL)) {
            cerr << "setitimer error." << '\n';
            return;
        }

        siglongjmp(newRunning->env, 2);
    }
}

void init_timer()
{
    // Install timer_handler as the signal handler for SIGVTALRM.
    sa.sa_handler = timer_handler;
    sa.sa_flags = 0;
    if (sigaction(SIGVTALRM, &sa, NULL) < 0)
    {
        cerr << "sigaction error." << '\n';
    }

    usec = quantumUsecs % CONVERT;
    sec = (quantumUsecs - usec) / CONVERT;

    // Configure the timer to expire after quantum_usecs sec... */
    timer.it_value.tv_sec = sec;        // first time interval, seconds part
    timer.it_value.tv_usec = usec;        // first time interval

    // configure the timer to expire every 3 sec after that.
    timer.it_interval.tv_sec = sec;    // following time intervals, seconds part
    timer.it_interval.tv_usec = usec;    // following time intervals, microseconds part

    // Start a virtual timer. It counts down whenever this process is executing.
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL))
    {
        cerr << "setitimer error." << '\n';
    }
}

int uthread_init(int quantum_usecs)
{
    if (quantum_usecs <= 0) {
        printThreadError("Quantum should be a positive integer");
        return -1;
    }
    quantumUsecs = quantum_usecs;
    tm = new ThreadManager(MAX_THREAD_NUM, quantumUsecs);
    init_timer();
    return SUCCESS;
}

int uthread_spawn(void (*f)(void))
{
    int idx = tm->addThread(f);
    if (idx == FAILURE) {
        printThreadError("Threads exceeded the max amount allowed.");
        return FAILURE;
    }
    return idx;
}

int uthread_terminate(int tid)
{
    // If a thread terminates itself or the main thread is terminated.
    if (tid == MAIN_THREAD) {
        exitProgram();
    }

    int result = tm->terminateThread(tid);
    if (result == FAILURE) {
        printThreadError(INVALID_IDX);
        return FAILURE;
    }

    return SUCCESS;
}


int uthread_block(int tid) {

    int result = tm->blockThread(tid);
    if (result == FAILURE) {
        printThreadError(INVALID_IDX);
        return FAILURE;
    }

    return SUCCESS;
}

int uthread_resume(int tid) {

    int result = tm->resumeThread(tid, false);
    if (result == FAILURE) {
        printThreadError(INVALID_IDX);
        return FAILURE;
    }

    return SUCCESS;
}

int uthread_sync(int tid) {
    int result = tm->syncThread(tid);
    if (result == FAILURE) {
        printThreadError(INVALID_IDX);
    }
    return result;
}

int uthread_get_tid() {

    return tm->runningThreadID();
}

int uthread_get_total_quantums() {
    return tm->getTotalQuantum();
}

int uthread_get_quantums(int tid){
    int res = tm->isThreadExist(tid);
    if (res == FAILURE){
        return FAILURE;
    }
    Thread *thread = tm->threads[tid];
    int quantums = thread->quantums;
    return quantums;
}






