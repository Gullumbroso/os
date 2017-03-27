//
// Created by Gilad Lumbroso on 21/03/2017.
//

#ifndef EX2_THREAD_H
#define EX2_THREAD_H

#include <stdio.h>
#include <unistd.h>
#include <signal.h>


#define READY 0
#define RUNNING 1
#define BLOCKED 2


class Thread
{
private:
    int state, id;
    void (*f)(void);
public:
    Thread(int id, void (*f)(void));
    ~Thread();

    int getId();
    int getState();

    bool operator==(const Thread &other) const;
    bool operator!=(const Thread &other) const;
};


#endif //EX2_THREAD_H
