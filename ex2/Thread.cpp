//
// Created by Gilad Lumbroso on 21/03/2017.
//

#include "Thread.h"



Thread::Thread(int id, void (*f)(void))
{
    this->state = READY;
    this->id = id;

    if (id == 0) {
        this->quantums = 1;
    } else {
        this->quantums = 0;
    }

    this->f = f;
    this->isSynced = false;
    this->isBlocked = false;
}

int Thread::getState()
{
    return state;
}

bool Thread::operator==(const Thread &other) const
{
    return this->id == other.id;
}

bool Thread::operator!=(const Thread &other) const
{
    return this->id != other.id;
}

void Thread::setState(int s) {
    state = s;
}

void Thread::sync(Thread *t) {
    synced.push_back(t);
}

Thread::~Thread()
{
}