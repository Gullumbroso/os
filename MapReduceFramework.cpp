//
// Created by Gilad Lumbroso on 08/05/2017.
//

#include "stdlib.h"
#include "vector"
#include "MapReduceFramework.h"
#include <pthread.h>
#include <semaphore.h>

using namespace std;
typedef pair<k2Base*, v2Base*> SHUFFLE_ITEM;
typedef vector<SHUFFLE_ITEM> SHUFFLE_VEC;



#define CHUNK_SIZE 10


IN_ITEMS_VEC k1v1Container;
SHUFFLE_VEC k2v2Container;
int k1v1Index = 0;
vector<pthread_t *> execMapThreads;
vector<void*> threadsArgs;
sem_t *semaphore;


void prepareMappingPhase(MapReduceBase &mapReduceBase, int multiThreadLevel);
void *mapExecFunc();


OUT_ITEMS_VEC
RunMapReduceFramework(MapReduceBase &mapReduce, IN_ITEMS_VEC &itemsVec, int multiThreadLevel,
                      bool autoDeleteV2K2)
{
    prepareMappingPhase(mapReduce, multiThreadLevel);
    k1v1Container = itemsVec;

    return OUT_ITEMS_VEC();
}

void prepareMappingPhase(MapReduceBase &mapReduceBase, int multiThreadLevel)
{
    // Create <multiThreadLevel> number of threads, if needed
    int numOfThreadsCount = 0;
    while (numOfThreadsCount < k1v1Container.size())
    {
        pthread_t thread;
        int res = pthread_create(&thread, NULL, mapExecFunc, &k1v1Index);
        if (res < 0)
        {
            
        }
    }
}

/**
 * @brief: The function that is run by the MapExec threads.
 */
void *mapExecFunc(void *initIdx)
{
    unsigned long containerSize = k1v1Container.size();

    while (k1v1Index < containerSize) {

        // Take a batch from the container
        int chunkSize = (int) min(containerSize - k1v1Index - 1, CHUNK_SIZE);
        vector<IN_ITEM>::const_iterator first = k1v1Container.begin() + k1v1Index;
        vector<IN_ITEM>::const_iterator last = k1v1Container.begin() + chunkSize;
        vector<IN_ITEM> chunk(first, last);

        // Run the map function on each element
        for (auto it = k1v1Container.begin(); it < k1v1Container.end(); it++)
        {

        }
    }
}


void shuffle(SHUFFLE_VEC &shuffle_vec){



    // if we don't need the semaphore anymore.
    sem_destroy(semaphore);
}