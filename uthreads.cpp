//
// Created by Gilad Lumbroso on 21/03/2017.
//

#include <stdlib.h>
#include <string>
#include <iostream>
#include "uthreads.h"
#include "ThreadManager.h"
using namespace std;

ThreadManager *tm;
int quantum;

void printThreadError(string message)
{
    cerr << "thread library error: "  << message << "\n";
}

void printSystemError(string message)
{
    cerr << "system error: "  << message << "\n";
}

int uthread_init(int quantum_usecs)
{
    if (quantum_usecs <= 0) {
        printThreadError("Quantum should be a positive integer");
        return -1;
    }
    quantum = quantum_usecs;
    tm = new ThreadManager(MAX_THREAD_NUM);
    return 0;
}

int uthread_spawn(void (*f)(void))
{
    int idx = tm->addThread(f);
    if (idx == -1) {
        printThreadError("Threads exceeded the max amount allowed.");
        return -1;
    }
    return idx;
}

int uthread_terminate(int tid)
{
    int result = tm->terminateThread(tid);
    if (result == -1) {
        printThreadError("Invalid index.");
        return -1;
    }
    return 0;
}






