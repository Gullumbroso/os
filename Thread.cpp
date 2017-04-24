//
// Created by Gilad Lumbroso on 21/03/2017.
//

#include "Thread.h"

Thread::Thread(int id, void (*f)(void))
{
    this->state = READY;
    this->id = id;
    this->quantums = 0;
    this->f = f;
    this->isSynced = false;
    this->isBlocked = false;
}

int Thread::getId()
{
    return id;
}

Thread::~Thread()
{

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

int Thread::saveState() {
    return sigsetjmp(env, 1);
}

void Thread::loadState() {
    siglongjmp(env, 1);
}

void Thread::sync(Thread &t) {
    synced.push_back(t);
}
