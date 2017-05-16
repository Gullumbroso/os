//
// Created by Gilad Lumbroso on 15/05/2017.
//

#ifndef PROJECT_EXECREDUCETHREAD_H
#define PROJECT_EXECREDUCETHREAD_H


#include "MapReduceFramework.h"
#include "Thread.h"
#include <pthread.h>

/**
 * @brief A class that represents an ExecReduce thread.
 */
class ExecReduceThread: public Thread
{
public:
    OUT_ITEMS_VEC container;

    ExecReduceThread();
    ~ExecReduceThread();
};


#endif //PROJECT_EXECREDUCETHREAD_H
