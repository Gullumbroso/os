//
// Created by Gilad Lumbroso on 21/03/2017.
//

#ifndef EX2_THREAD_H
#define EX2_THREAD_H

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <vector>

using namespace std;


#define READY 0
#define BLOCKED 2


class Thread
{
private:
    int state, id;
    vector<Thread> synced;

public:
    Thread(int id, void (*f)(void));
    ~Thread();

    void (*f)(void);
    sigjmp_buf env;

    int getId();
    int getState();
    void setState(int s);
    void sync(Thread &t);

    void saveState();
    void loadState();

    bool operator==(const Thread &other) const;
    bool operator!=(const Thread &other) const;
};


#endif //EX2_THREAD_H
