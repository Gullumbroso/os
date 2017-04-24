//
// Created by Gilad Lumbroso on 21/03/2017.
//

#include <stdlib.h>
#include <string>
#include <iostream>
#include "uthreads.h"
#include "ThreadManager.h"

using namespace std;


#define SUCCESS 0
#define FAILURE -1
#define MAIN_THREAD 0
#define INVALID_IDX "Invalid index."


ThreadManager *tm;

void printThreadError(string message)
{
    cerr << "thread library error: "  << message << "\n";
}

void printSystemError(string message)
{
    cerr << "system error: "  << message << "\n";
}

void exitProgram() {
    exit(0);
}

int uthread_init(int quantum_usecs)
{

    if (quantum_usecs <= 0) {
        printThreadError("Quantum should be a positive integer");
        return -1;
    }
    tm = new ThreadManager(MAX_THREAD_NUM, quantum_usecs);
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
    try {
        if (tid == MAIN_THREAD) {
            exitProgram();
        }
    }
    catch (int e) {
        if (e == MAIN_THREAD) {
            // The main thread was terminated
            exitProgram();
            throw e;
        }
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
    return tm->threads[tid].quantums;
}






