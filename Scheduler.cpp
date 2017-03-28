//
// Created by gullumbroso on 3/27/17.
//

#include "Scheduler.h"

void Scheduler::timer_handler(int sig)
{
    // Time for thread has passed
    nextStep();
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

Scheduler::Scheduler(int quantum_usecs)
{
    quantum = 1;
    quantumUsecs = quantum_usecs;
    init_timer();
}

void Scheduler::nextStep()
{

}

