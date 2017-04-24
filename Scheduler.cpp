//
// Created by gullumbroso on 3/27/17.
//

#include "Scheduler.h"

void Scheduler::timer_handler(int sig)
{
    // Move the current thread to the back of the Ready list after saving its state
    auto readyThreads = threadManager->readyThreads;
    Thread runningThread = readyThreads[0];
    runningThread.saveState();
    readyThreads.erase(readyThreads.begin());
    readyThreads.push_back(runningThread);

    // Check what thread should be run now
    if (threadManager->syncThreadsCounter > 0) {
        auto blockedThreads = threadManager->blockedThreads;
        Thread nextThread = blockedThreads[0];
        blockedThreads.erase(blockedThreads.begin());
        readyThreads.insert(readyThreads.begin(), nextThread);
        threadManager->syncThreadsCounter--;

    } else {
        auto
    }
}

void Scheduler::init_timer()
{

    // Install timer_handler as the signal handler for SIGVTALRM.
    sa.sa_handler = Scheduler::timer_handler;
    if (sigaction(SIGVTALRM, &sa, NULL) < 0)
    {
        cerr << "sigaction error." << '\n';
    }

    // Configure the timer to expire after quantum_usecs sec... */
    timer.it_value.tv_sec = 0;        // first time interval, seconds part
    timer.it_value.tv_usec = quantumUsecs;        // first time interval

    // configure the timer to expire every 3 sec after that.
    timer.it_interval.tv_sec = 0;    // following time intervals, seconds part
    timer.it_interval.tv_usec = quantumUsecs;    // following time intervals, microseconds part

    // Start a virtual timer. It counts down whenever this process is executing.
    if (setitimer(ITIMER_VIRTUAL, &timer, NULL))
    {
        cerr << "setitimer error." << '\n';
    }
}

Scheduler::Scheduler(int quantum_usecs, ThreadManager *tm)
{
    quantum = 1;
    quantumUsecs = quantum_usecs;
    threadManager = tm;
    init_timer();
}

void Scheduler::nextStep()
{

}

void Scheduler::setQuantom() {
    this->quantum += 1;
}

int Scheduler::getQuantum() {
    return this->quantum;
}

