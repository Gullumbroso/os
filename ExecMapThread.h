//
// Created by gullumbroso on 5/9/17.
//

#ifndef PROJECT_EXECMAP_H
#define PROJECT_EXECMAP_H

#include <stdlib.h>
#include <vector>
#include <pthread.h>
#include "MapReduceClient.h"

using namespace std;

typedef pair<k2Base*, v2Base*> SHUFFLE_ITEM;
typedef vector<SHUFFLE_ITEM> SHUFFLE_VEC;
typedef pair<k2Base*,vector<v2Base*>> SHUFFLE_RET;
typedef vector<SHUFFLE_RET> SHUFFLE_RET_VEC;

/**
 * @brief A class that represents an ExecMap thread.
 */
class ExecMapThread {
public:

    pthread_t thread;
    SHUFFLE_VEC container;
    pthread_mutex_t containerMutex;

    ExecMapThread();

    ~ExecMapThread();
};


#endif //PROJECT_EXECMAP_H
