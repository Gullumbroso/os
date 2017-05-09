//
// Created by Gilad Lumbroso on 08/05/2017.
//

#include "stdlib.h"
#include "stdio.h"
#include "string"
#include "vector"
#include "MapReduceFramework.h"
#include <pthread.h>
#include <iostream>

using namespace std;

#define SUCCESS 0
#define FAILURE 1
#define CHUNK_SIZE 10


// GLOBAL VARIABLES
IN_ITEMS_VEC k1v1Container, k2v2Container;

vector<pthread_t *> execMapThreads;
vector<void*> threadsArgs;

int k1v1Index;
pthread_mutex_t k1v1Mutex;

// METHODS
void prepareMappingPhase(MapReduceBase &mapReduceBase, int multiThreadLevel);
void *mapExecFunc();


/**
 * @brief Prints an error message to the standard error with the name of the failing function.
 * @param failingFunc The name of the failing function.
 */
void exitWithError(string failingFunc)
{
    cerr << "MapReduceFramework Failure: " << failingFunc << " failed." << endl;
    exit(FAILURE);
}


/**
 * @brief Initializes global variables
 */
void init()
{
    k1v1Index = 0;
    k1v1Mutex = PTHREAD_MUTEX_INITIALIZER;

}


OUT_ITEMS_VEC
RunMapReduceFramework(MapReduceBase &mapReduce, IN_ITEMS_VEC &itemsVec, int multiThreadLevel,
                      bool autoDeleteV2K2)
{
    init();
    prepareMappingPhase(mapReduce, multiThreadLevel);
    k1v1Container = itemsVec;

    return OUT_ITEMS_VEC();
}

void prepareMappingPhase(MapReduceBase &mapReduceBase, int multiThreadLevel)
{
    // Create <multiThreadLevel> number of threads, if needed
    int numOfThreadsCount = 0;
    while (numOfThreadsCount < k1v1Container.size() && (int) execMapThreads.size < multiThreadLevel)
    {
        pthread_t thread;
        int res = pthread_create(&thread, NULL, mapExecFunc, &mapReduceBase);
        if (res < 0)
        {
            exitWithError("pthread_create");
        }
        execMapThreads.push_back(&thread);
        numOfThreadsCount += CHUNK_SIZE;
    }
}

/**
 * @brief The function that is run by the MapExec threads.
 */
void *mapExecFunc(void *mrb)
{
    MapReduceBase *mapReduceBase = (MapReduceBase *) mrb;
    unsigned long containerSize = k1v1Container.size();

    while (k1v1Index < containerSize) {

        // Take a batch from the container
        int chunkSize = (int) min(containerSize - k1v1Index, CHUNK_SIZE);
        vector<IN_ITEM>::const_iterator first = k1v1Container.begin() + k1v1Index;
        vector<IN_ITEM>::const_iterator last = k1v1Container.begin() + chunkSize;
        vector<IN_ITEM> chunk(first, last);

        // Run the map function on each element
        for (auto it = chunk.begin(); it < chunk.end(); it++)
        {
            IN_ITEM pair = *it;
            mapReduceBase->Map(pair.first, pair.second);
        }
    }

    pthread_exit(NULL);
}
