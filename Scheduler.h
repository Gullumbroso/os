//
// Created by gullumbroso on 3/27/17.
//

#ifndef EX2_SCHEDULER_H
#define EX2_SCHEDULER_H


#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sys/time.h>
#include <signal.h>

using namespace std;


class Scheduler {
private:
    int quantum, quantumUsecs;
    static bool timePassed;
    struct sigaction sa;
    struct itimerval timer;

    void timer_handler(int timePassed);
    void init_timer();

public:
    Scheduler(int quantum_usecs);
    int getQuantum();
    void nextStep();
    ~Scheduler();
};


#endif //EX2_SCHEDULER_H
