//
// Created by gullumbroso on 3/27/17.
//

#ifndef EX2_SCHEDULER_H
#define EX2_SCHEDULER_H


#include <sys/time.h>
#include <signal.h>


class Scheduler {
private:
    int quantum;
    bool timePassed;
    struct sigaction sa;
    struct itimerval timer;

    void timer_handler();
    void init_timer(int quantum_usecs);

public:
    Scheduler(int quantum_usecs);
    int getQuantum();
    void nextStep();
    ~Scheduler();
};


#endif //EX2_SCHEDULER_H
