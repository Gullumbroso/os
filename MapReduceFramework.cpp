//
// Created by Gilad Lumbroso on 08/05/2017.
//

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "MapReduceFramework.h"
#include "ExecMapThread.h"
#include <iostream>
#include <semaphore.h>
#include <map>

using namespace std;


#define SUCCESS 0
#define FAILURE 1
#define CHUNK_SIZE 10


// GLOBAL VARIABLES
IN_ITEMS_VEC k1v1Container;
SHUFFLE_VEC k2v2Container;
SHUFFLE_RET_VEC shuffleRetVec;
pthread_t shuffleThread;


vector<ExecMapThread *> execMapThreads;
vector<void*> threadsArgs;
sem_t shuffleSem;

int k1v1Index;

map<pthread_t, ExecMapThread *> pthreadToThreadObject;

// MUTEXES
pthread_mutex_t k1v1_mutex;
pthread_mutex_t pthreadToContainer_mutex;

// METHODS
void prepareMappingPhase(MapReduceBase &mapReduceBase, int multiThreadLevel);
void *mapExecFunc();
void shuffleAdd(k2Base *k2, v2Base *v2);


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
    k1v1_mutex = PTHREAD_MUTEX_INITIALIZER;

    // Create the shuffleFunc thread
    sem_init(&shuffleSem, 0, 1);
    int res = pthread_create(&shuffleThread, NULL, shuffleFunc, &k2v2Container);
    if (res < 0)
    {
        exitWithError("pthread_create");
    }

//    // Create the execMapContainers mutex
//    execMapContainers_mutex = PTHREAD_MUTEX_INITIALIZER;

    // Create and lock the ptheradToContainer_mutex
    pthreadToContainer_mutex = PTHREAD_MUTEX_INITIALIZER;
    res = pthread_mutex_lock(&pthreadToContainer_mutex);
    if (res < 0)
    {
        exitWithError("pthread_mutex_lock");
    }
}


OUT_ITEMS_VEC
RunMapReduceFramework(MapReduceBase &mapReduce, IN_ITEMS_VEC &itemsVec, int multiThreadLevel,
                      bool autoDeleteV2K2)
{
    init();
    prepareMappingPhase(mapReduce, multiThreadLevel);

    int res = pthread_mutex_unlock(&pthreadToContainer_mutex);
    if (res < 0)
    {
        exitWithError("pthread_mutex_lock");
    }

    k1v1Container = itemsVec;

    return OUT_ITEMS_VEC();
}


void prepareMappingPhase(MapReduceBase &mapReduceBase, int multiThreadLevel)
{
    // Create <multiThreadLevel> number of threads, if needed
    int numOfThreadsCount = 0;

    while (numOfThreadsCount < k1v1Container.size() && (int) execMapThreads.size < multiThreadLevel)
    {
        ExecMapThread *t = new ExecMapThread();

        int res = pthread_create(&(t->thread), NULL, mapExecFunc, &mapReduceBase);
        if (res < 0)
        {
            exitWithError("pthread_create");
        }

        execMapThreads.push_back(t);

        numOfThreadsCount += CHUNK_SIZE;

        // Creating the pthreadToThreadObject map
        pthreadToThreadObject.insert(pair<pthread_t, ExecMapThread *>(t->thread, t));
    }
}

/**
 * @brief The function that is run by the MapExec threads.
 */
void *mapExecFunc(void *mrb)
{
    // Waiting for the ptheradToContainer map to initialize
    int res = pthread_mutex_lock(&pthreadToContainer_mutex);
    if (res < 0)
    {
        exitWithError("pthread_mutex_lock");
    }
    res = pthread_mutex_unlock(&pthreadToContainer_mutex);
    if (res < 0)
    {
        exitWithError("pthread_mutex_unlock");
    }

    MapReduceBase *mapReduceBase = (MapReduceBase *) mrb;
    unsigned long containerSize = k1v1Container.size();

    while (k1v1Index < containerSize) {

        res = pthread_mutex_lock(&k1v1_mutex);
        if (res < 0)
        {
            exitWithError("pthread_mutex_lock");
        }

        // Check if the k1v1Index value hasn't been modified since the entrance to the loop
        if (k1v1Index >= containerSize)
        {
            break;
        }

        int chunkSize = (int) min(containerSize - k1v1Index, CHUNK_SIZE);
        k1v1Index += chunkSize;

        pthread_mutex_unlock(&k1v1_mutex);

        // Take a batch from the container
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

/**
 * @brief The library funciton the emits a <k2,v2> pair into the container of the current thread.
 * @param k2Base A pointer to the k2base.
 * @param v2Base A pointer to the v2base.
 */
void Emit2(k2Base *k2Base, v2Base *v2Base)
{
    pthread_t threadID = pthread_self();
    ExecMapThread *t = pthreadToThreadObject.at(threadID);
    SHUFFLE_VEC *container = &(t->container);

    // Lock the container before the addition
    int res = pthread_mutex_lock(&(t->containerMutex));
    if (res < 0)
    {
        exitWithError("pthread_mutex_lock");
    }

    container->push_back(pair(k2Base, v2Base));

    res = pthread_mutex_lock(&(t->containerMutex));
    if (res < 0)
    {
        exitWithError("pthread_mutex_unlock");
    }

    res = sem_post(&shuffleSem);

    if (res < 0)
    {
        exitWithError("sem_post");
    }
}

/**
 * @brief The function that is run by the shuffle thread.
 * @param args The function's arguments.
 */
void *shuffleFunc(void *args) {

    int res = sem_wait(&shuffleSem);
    if (res < 0)
    {
        exitWithError("sem_wait");
    }

    for (auto it = execMapThreads.begin(); it < execMapThreads.end(); it++)
    {
        ExecMapThread *t = *it;
        SHUFFLE_VEC *container = &(t->container);
        if (!t->container.empty())
        {
            SHUFFLE_ITEM pair;
            res = pthread_mutex_lock(&(t->containerMutex));
            if (res < 0)
            {
                exitWithError("pthread_mutex_lock");
            }

            pair = container->back();
            container->pop_back();

            res = pthread_mutex_unlock(&(t->containerMutex));
            if (res < 0)
            {
                exitWithError("pthread_mutex_unlock");
            }

            shuffleAdd(pair.first, pair.second);
        }
    }

    pthread_exit(NULL);
}


void Emit3(k3Base *k3Base, v3Base *v3Base) {

}

void shuffleAdd(k2Base *k2, v2Base *v2){
    bool addPair = true;
    for(auto it = shuffleRetVec.begin(); it < shuffleRetVec.end() ; it++){
        SHUFFLE_RET pair = *it;
        if(!(*k2 < *pair.first) && !(*pair.first < *k2)){
            pair.second.push_back(v2);
            *it = pair;
            addPair = false;
        }
    }
    if(addPair){
        SHUFFLE_RET addShuff;
        addShuff.first = k2;
        vector<v2Base*> addVec;
        addVec.push_back(v2);
        addShuff.second = addVec;
        shuffleRetVec.push_back(addShuff);
    }
}


// TODO: delete and destroy all the resources that we used.