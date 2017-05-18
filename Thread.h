//
// Created by Gilad Lumbroso on 15/05/2017.
//

#ifndef PROJECT_THREAD_H
#define PROJECT_THREAD_H


#include "MapReduceClient.h"
#include <stdlib.h>
#include <vector>
#include <pthread.h>

using namespace std;

typedef pair<k2Base*, v2Base*> SHUFFLE_ITEM;
typedef vector<SHUFFLE_ITEM> SHUFFLE_VEC;
typedef pair<k2Base*,vector<v2Base*>> SHUFFLE_RET;
typedef vector<SHUFFLE_RET> SHUFFLE_RET_VEC;


class Thread
{
public:

    pthread_t thread;
    pthread_mutex_t containerMutex;

    Thread();

    virtual ~Thread();
};


#endif //PROJECT_THREAD_H
