//
// Created by Gilad Lumbroso on 15/05/2017.
//

#include "Thread.h"

Thread::Thread()
{
    containerMutex = PTHREAD_MUTEX_INITIALIZER;
}

Thread::~Thread()
{
    pthread_mutex_destroy(&containerMutex);
}


