//
// Created by gullumbroso on 3/27/17.
//

#include "Scheduler.h"

void Scheduler::timer_handler()
{
    timePassed = true;
}

void init_timer(int quantum_usecs) {

    // Install timer_handler as the signal handler for SIGVTALRM.
    sa.sa_handler = &timer_handler;
    if (sigaction(SIGVTALRM, &sa,NULL) < 0) {
        printf("sigaction error.");
    }
}

Scheduler::Scheduler(int quantum_usecs) {
    init_timer(quantum_usecs);
}
