//
// Created by Gilad Lumbroso on 21/03/2017.
//

#include "Thread.h"

Thread::Thread(int id, void (*f)(void))
{
    this->state = READY;
    this->id = id;
    this->f = f;
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