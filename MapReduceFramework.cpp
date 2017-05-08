//
// Created by Gilad Lumbroso on 08/05/2017.
//

#include "stdlib.h"
#include "vector"
#include "MapReduceFramework.h"
#include <pthread.h>

using namespace std;

#define CHUNCK 10


vector<pthread_t *> execMapThreads;


void prepareMappingPhase(MapReduceBase &mapReduceBase, int multiThreadLevel);


OUT_ITEMS_VEC
RunMapReduceFramework(MapReduceBase &mapReduce, IN_ITEMS_VEC &itemsVec, int multiThreadLevel,
                      bool autoDeleteV2K2)
{
    prepareMappingPhase(mapReduce, multiThreadLevel);


    return OUT_ITEMS_VEC();
}

void prepareMappingPhase(MapReduceBase $mapReduceBase, int multiThreadLevel)
{
    // Create <multiThreadLevel> number of threads
    for (int i = 0; i < multiThreadLevel; i++)
    {
        pthread_t *thread;
        int res = pthread_create(thread, NULL, )
    }
}
