//
// Created by gullumbroso on 3/27/17.
//

#ifndef EX2_SCHEDULER_H
#define EX2_SCHEDULER_H

#include <sys/time.h>

class Scheduler {
private:
    int quantum;
    bool timePassed;
    void timer_handler();
    struct itimerval timer;

public:
    Scheduler(int quantum_usecs);
    int getQuantum();
    void nextStep();
    ~Scheduler();
};


#endif //EX2_SCHEDULER_H
