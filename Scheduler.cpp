//
// Created by gullumbroso on 3/27/17.
//

#include "Scheduler.h"

static void timer_handler(int timePassed)
{
    timePassed = true;
}

void Scheduler::init_timer(int quantum_usecs) {

    // Install timer_handler as the signal handler for SIGVTALRM.
    sa.sa_handler = Scheduler::timer_handler;
    if (sigaction(SIGVTALRM, &sa,NULL) < 0) {

    }
}

Scheduler::Scheduler(int quantum_usecs) {
    init_timer(quantum_usecs);
}
