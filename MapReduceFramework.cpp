//
// Created by Gilad Lumbroso on 08/05/2017.
//

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <semaphore.h>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <sys/time.h>
#include "MapReduceFramework.h"
#include "ExecMapThread.h"
#include "ExecReduceThread.h"

using namespace std;


#define SUCCESS 0
#define FAILURE 1
#define CHUNK_SIZE 10
#define LOG_FILE_NAME "/cs/usr/avishadler/Desktop/MapReduceFramework.log"
#define MAP_THREAD_NAME "ExecMap"
#define SHUFFLE_THREAD_NAME "Shuffle"
#define REDUCE_THREAD_NAME "ExecReduce"


// GLOBAL VARIABLES

IN_ITEMS_VEC k1v1Container;
SHUFFLE_RET_VEC shuffleRetVec;
OUT_ITEMS_VEC finalOutput;


pthread_t shuffleThread;
vector<ExecMapThread *> execMapThreads;
vector<ExecReduceThread *> execReduceThreads;

sem_t shuffleSem;

ofstream logFile;

unsigned long k1v1Index;
unsigned long shuffleIndex;
int unfinishedThreadsCounter;

bool autoDelete;

map<pthread_t, Thread *> pthreadToThreadObject;

struct timeval startTimer, endTimer;


// MUTEXES

pthread_mutex_t k1v1_mutex;
pthread_mutex_t unfinishedThreads_mutex;
pthread_mutex_t shuffleCreate_mutex;
pthread_mutex_t shuffle_mutex;
pthread_mutex_t pthreadToContainer_mutex;
pthread_mutex_t logFile_mutex;


// METHODS

void mappingPhase(MapReduceBase &mapReduceBase, int multiThreadLevel);

void reducingPhase(MapReduceBase &mapReduceBase, int multiThreadLevel);

void shuffleAdd(k2Base *k2, v2Base *v2);

void printToLog(string content);

void releaseResources(bool autoDeleteV2K2);

void *shuffleFunc(void *args);

void *execMapFunc(void *mrb);

void *execReduceFunc(void *mrb);


// TODO: Delete this method after debugging
pthread_mutex_t printMutex = PTHREAD_MUTEX_INITIALIZER;
void safePrint(string msg) {
    pthread_mutex_lock(&printMutex);
    cout << msg << endl;
    pthread_mutex_unlock(&printMutex);
}


/**
 * @brief Prints an error message to the standard error with the name of the failing function.
 * @param failingFunc The name of the failing function.
 */
void exitWithError(string failingFunc)
{
    cerr << "MapReduceFramework Failure: " << failingFunc << " failed." << endl;
    releaseResources(autoDelete);
    exit(FAILURE);
}

/**
 * Compares two OUT_ITEM elements
 */
inline bool compareOutItem(const OUT_ITEM &first, const OUT_ITEM &second) {
    return first.first-> operator< (*second.first);
}

/**
 * @return The current date and time as a string.
 */
string getCurrentTime()
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    ostringstream oss;
    oss << std::put_time(&tm, "%d.%m.%Y %H:%M:%S");
    return oss.str();
}

/**
 * @return The elapsed time since and.
 */
double getElapsedTime()
{
    double time = (endTimer.tv_sec * 1000000 + endTimer.tv_usec) -
            (startTimer.tv_sec * 1000000 + startTimer.tv_usec);
    return time * 1000;
}


/**
 * @brief Initializes global variables
 */
void init()
{
    k1v1Index = 0;
    shuffleIndex = 0;

    k1v1_mutex = PTHREAD_MUTEX_INITIALIZER;
    shuffle_mutex = PTHREAD_MUTEX_INITIALIZER;
    shuffleCreate_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthreadToContainer_mutex = PTHREAD_MUTEX_INITIALIZER;
    unfinishedThreads_mutex = PTHREAD_MUTEX_INITIALIZER;
    logFile_mutex = PTHREAD_MUTEX_INITIALIZER;

    sem_init(&shuffleSem, 0, 0);

    logFile.open(LOG_FILE_NAME);
}


/**
 * @brief Creates the shuffle thread.
 */
void createShuffleThread()
{
    int res = pthread_mutex_lock(&shuffleCreate_mutex);
    if (res < 0)
    {
        exitWithError("pthread_mutex_lock");
    }

    res = pthread_create(&shuffleThread, NULL, shuffleFunc, NULL);
    if (res < 0)
    {
        exitWithError("pthread_create");
    }
    // Print to the log file
    string printStr = "Thread "
                      + string(SHUFFLE_THREAD_NAME)
                      + " created ["
                      + string(getCurrentTime())
                      + "]";
    printToLog(printStr);
}


OUT_ITEMS_VEC
RunMapReduceFramework(MapReduceBase &mapReduce, IN_ITEMS_VEC &itemsVec, int multiThreadLevel,
                      bool autoDeleteV2K2)
{
    autoDelete = autoDeleteV2K2;
    k1v1Container = itemsVec;
    unfinishedThreadsCounter = multiThreadLevel;

    init();

    printToLog("RunMapReduceFramework started with " + string(to_string(multiThreadLevel)) + " "
            "threads");

    createShuffleThread();

    // Block the main thread until the shuffle is created
    int res = pthread_mutex_lock(&shuffleCreate_mutex);
    if (res < 0)
    {
        exitWithError("pthread_mutex_lock");
    }
    res = pthread_mutex_unlock(&shuffleCreate_mutex);
    if (res < 0)
    {
        exitWithError("pthread_mutex_unlock");
    }

    safePrint("Starting the mapping and shuffle phase");

    // Prepare and run the ExecMap threads and the Shuffle thread
    gettimeofday(&startTimer, NULL);
    mappingPhase(mapReduce, multiThreadLevel);

    // Block the main thread until all the ExecReduce threads finished their job
    for (auto it = execMapThreads.begin(); it < execMapThreads.end(); it++)
    {
        ExecMapThread *mapThread = *it;
        pthread_join(mapThread->thread, NULL);
    }

    // Block the main thread until the shuffle finished its job
    res = pthread_cancel(shuffleThread);
    if (res < 0)
    {
        exitWithError("pthread_cancel");
    }

    res = pthread_join(shuffleThread, NULL);
    if (res < 0)
    {
        exitWithError("pthread_join");
    }

    safePrint("Starting the reducing phase");

    gettimeofday(&endTimer, NULL);
    printToLog("Map and Shuffle took " + string(to_string(getElapsedTime())) + "ns");

    // By now the shuffle phase is finished - it's time to move on to the reduce phase
    gettimeofday(&startTimer, NULL);
    reducingPhase(mapReduce, multiThreadLevel);
    gettimeofday(&endTimer, NULL);
    printToLog("Reduce took " + string(to_string(getElapsedTime())) + "ns");

    // Block the main thread until all the ExecReduce threads finished their job
    for (auto it = execReduceThreads.begin(); it < execReduceThreads.end(); it++)
    {
        ExecReduceThread *reduceThread = *it;
        pthread_join(reduceThread->thread, NULL);
    }

    // Merge the contents of the reduce containers into the final output container
    for (auto it = execReduceThreads.begin(); it < execReduceThreads.end(); it++)
    {
        ExecReduceThread *thread = *it;
        finalOutput.insert(finalOutput.begin(), thread->container.begin(), thread->container.end());
    }

    sort(finalOutput.begin(), finalOutput.end(), compareOutItem);

    printToLog("RunMapReduceFramework finished :)))");

    releaseResources(autoDelete);

    return finalOutput;
}

/**
 * @brief Prepares all the necessary resources for the mapping phase.
 * @param mapReduceBase The map reduce base object.
 * @param multiThreadLevel The number of threads that should be run simultinously.
 */
void mappingPhase(MapReduceBase &mapReduceBase, int multiThreadLevel)
{
    // TODO: Delete the num of threads count if we don't use it
    int numOfThreadsCount = 0;

    // Lock the ptheradToContainer_mutex so the ExecMap threads will wait for the
    // initialization of the map
    int res = pthread_mutex_lock(&pthreadToContainer_mutex);
    if (res < 0)
    {
        exitWithError("pthread_mutex_lock");
    }

    while (((int) execMapThreads.size() < multiThreadLevel)
//           && (numOfThreadsCount < (int) k1v1Container.size())
           )
    {
        ExecMapThread *t = new ExecMapThread(autoDelete);
        if (t == nullptr)
        {
            exitWithError("new");
        }

        res = pthread_create(&(t->thread), NULL, execMapFunc, &mapReduceBase);
        if (res < 0)
        {
            exitWithError("pthread_create");
        }

        // Print to the log file
        string printStr = "Thread "
                          + string(MAP_THREAD_NAME)
                          + " created ["
                          + string(getCurrentTime())
                          + "]";
        printToLog(printStr);

        execMapThreads.push_back(t);

        numOfThreadsCount += CHUNK_SIZE;

        // Creating the pthreadToThreadObject map
        pthreadToThreadObject.insert(pair<pthread_t, Thread *>(t->thread, t));
    }

    // The map is ready - unlock the ExecMap threads
    res = pthread_mutex_unlock(&pthreadToContainer_mutex);
    if (res < 0)
    {
        exitWithError("pthread_mutex_unlock");
    }
}

/**
 * @brief The function that is run by the ExecMap threads.
 */
void *execMapFunc(void *mrb)
{
    // Waiting for the pthreadToContainer map to initialize
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

    while (k1v1Index < containerSize)
    {
        string(to_string((pthread_self())));

        res = pthread_mutex_lock(&k1v1_mutex);
        if (res < 0)
        {
            exitWithError("pthread_mutex_lock");
        }

        long chunkSize = (long) min((long) (containerSize - k1v1Index), (long) CHUNK_SIZE);
        long k1v1LocalIndex = k1v1Index;
        k1v1Index += chunkSize;

        res = pthread_mutex_unlock(&k1v1_mutex);
        if (res < 0)
        {
            exitWithError("pthread_mutex_unlock");
        }

        // Take a batch from the container
        vector<IN_ITEM>::const_iterator first = k1v1Container.begin() + k1v1LocalIndex;
        vector<IN_ITEM>::const_iterator last = k1v1Container.begin() + k1v1LocalIndex + chunkSize;
        vector<IN_ITEM> chunk(first, last);

        // Run the map function on each element
        for (auto it = chunk.begin(); it < chunk.end(); it++)
        {
            IN_ITEM pair = *it;
            mapReduceBase->Map(pair.first, pair.second);
        }
    }

    // Decrement the unfinishedThreads counter
    res = pthread_mutex_lock(&unfinishedThreads_mutex);
    if (res < 0)
    {
        exitWithError("pthread_mutex_lock");
    }
    unfinishedThreadsCounter--;
    res = pthread_mutex_unlock(&unfinishedThreads_mutex);
    if (res < 0)
    {
        exitWithError("pthread_mutex_unlock");
    }

    // Just in case - increment the semaphore
    res = sem_post(&shuffleSem);
    if (res < 0)
    {
        exitWithError("sem_post");
    }

    // Print to the log file
    string printStr = "Thread "
                      + string(MAP_THREAD_NAME)
                      + " terminated ["
                      + string(getCurrentTime())
                      + "]";
    printToLog(printStr);

    pthread_exit(NULL);
}

/**
 * @brief The library funciton the emits a <k2,v2> pair into the container of the current thread.
 * @param k2Base A pointer to the k2base.
 * @param v2Base A pointer to the v2base.
 */
void Emit2(k2Base *k2, v2Base *v2)
{
    pthread_t threadID = pthread_self();

    ExecMapThread *t = (ExecMapThread *) pthreadToThreadObject.at(threadID);
    SHUFFLE_VEC *container = &(t->container);

    // Lock the container before the addition
    int res = pthread_mutex_lock(&(t->containerMutex));
    if (res < 0)
    {
        exitWithError("pthread_mutex_lock");
    }

    container->push_back(make_pair(k2, v2));

    res = pthread_mutex_unlock(&(t->containerMutex));
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
void *shuffleFunc(void*)
{
    // Unlock the mutex that blocks the main thread from creating the ExecMap threads
    int res = pthread_mutex_unlock(&shuffleCreate_mutex);
    if (res < 0)
    {
        exitWithError("pthread_mutex_unlock");
    }

    res = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    if (res < 0)
    {
        exitWithError("pthread_setcancelstate");
    }

    int semValue;
    res = sem_getvalue(&shuffleSem, &semValue);
    if (res < 0)
    {
        exitWithError("sem_getvalue");
    }

//    res = sem_post(&shuffleSem);
//    if (res < 0)
//    {
//        exitWithError("sem_post");
//    }

    while (unfinishedThreadsCounter > 0 || semValue > 0)
    {
        // Wait for new <k2,v2> pairs to deal with
        res = sem_wait(&shuffleSem);
        if (res < 0)
        {
            exitWithError("sem_wait");
        }
        res = sem_getvalue(&shuffleSem, &semValue);
        if (res < 0)
        {
            exitWithError("sem_getvalue");
        }

        for (auto it = execMapThreads.begin(); it < execMapThreads.end(); it++)
        {
            ExecMapThread *t = *it;
            SHUFFLE_VEC *container = &(t->container);
            SHUFFLE_VEC *containerToDelete = &(t->containerToDelete);
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
                containerToDelete->push_back(pair);

                res = pthread_mutex_unlock(&(t->containerMutex));
                if (res < 0)
                {
                    exitWithError("pthread_mutex_unlock");
                }

                shuffleAdd(pair.first, pair.second);

                break;
            }
        }
    }

    res = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (res < 0)
    {
        exitWithError("pthread_setcancelstate");
    }
}

/**
 * @brief Adds a v2Base to the vector of k2Base if exists, otherwise creates a new entry in the
 * shuffleRetVec.
 * @param k2 The key.
 * @param v2 The value.
 */
void shuffleAdd(k2Base *k2, v2Base *v2)
{
    bool addPair = true;
    for (auto it = shuffleRetVec.begin(); it < shuffleRetVec.end(); it++)
    {
        SHUFFLE_RET pair = *it;
        if (!(*k2 < *pair.first) && !(*pair.first < *k2))
        {
            pair.second.push_back(v2);
            *it = pair;
            addPair =  false;
            break;
        }
    }
    if (addPair)
    {
        SHUFFLE_RET addShuff;
        addShuff.first = k2;
        vector<v2Base *> addVec;
        addVec.push_back(v2);
        addShuff.second = addVec;
        shuffleRetVec.push_back(addShuff);
    }
}


/**
 * @brief Prepares all the necessary resources for the reducing phase.
 * @param mapReduceBase The map reduce base object.
 * @param multiThreadLevel The number of threads that should be run simultinously.
 */
void reducingPhase(MapReduceBase &mapReduceBase, int multiThreadLevel)
{
    int numOfThreadsCount = 0;

    // Lock the ptheradToContainer_mutex so the ExecReduce threads will wait for the
    // initialization of the map
    int res = pthread_mutex_lock(&pthreadToContainer_mutex);
    if (res < 0)
    {
        exitWithError("pthread_mutex_lock");
    }

    while ((int) execReduceThreads.size() < multiThreadLevel
//           && numOfThreadsCount < shuffleRetVec.size()
           )
    {
        safePrint("In the while loop of creation of new threads");

        ExecReduceThread *t = new ExecReduceThread();
        if (t == nullptr)
        {
            exitWithError("new");
        }

        res = pthread_create(&(t->thread), NULL, execReduceFunc, &mapReduceBase);
        if (res < 0)
        {
            exitWithError("pthread_create");
        }

        execReduceThreads.push_back(t);

        // Print to the log file
        string printStr = "Thread "
                          + string(REDUCE_THREAD_NAME)
                          + " created ["
                          + string(getCurrentTime())
                          + "]";
        printToLog(printStr);

        numOfThreadsCount += CHUNK_SIZE;

        // Updating the pthreadToThreadObject map
        pthreadToThreadObject.insert(pair<pthread_t, Thread *>(t->thread, t));
    }

    // The map is ready - unlock the ExecReduce threads
    res = pthread_mutex_unlock(&pthreadToContainer_mutex);
    if (res < 0)
    {
        exitWithError("pthread_mutex_unlock");
    }
    safePrint("Unlocked the pthreadToContainer_mutex and finished the creation of the threads.");
}


/**
 * @brief The function that is run by the ExecReduce threads.
 */
void *execReduceFunc(void *mrb)
{
    // Waiting for the pthreadToContainer map to initialize with the new reduce threads
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
    unsigned long containerSize = shuffleRetVec.size();

    while (shuffleIndex < containerSize)
    {

        res = pthread_mutex_lock(&shuffle_mutex);
        if (res < 0)
        {
            exitWithError("pthread_mutex_lock");
        }

        // Check if the shuffleIndex value hasn't been modified since the entrance to the loop
        if (shuffleIndex >= containerSize)
        {
            break;
        }

        long chunkSize = (long) min((long) (containerSize - shuffleIndex), (long) CHUNK_SIZE);
        long shuffleLocalIndex = shuffleIndex;
        shuffleIndex += chunkSize;

        pthread_mutex_unlock(&shuffle_mutex);

        // Take a batch from the container
        vector<SHUFFLE_RET>::const_iterator first = shuffleRetVec.begin() + shuffleLocalIndex;
        vector<SHUFFLE_RET>::const_iterator last = shuffleRetVec.begin() + shuffleLocalIndex + chunkSize;
        vector<SHUFFLE_RET> chunk(first, last);

        // Run the reduce function on each element
        for (auto it = chunk.begin(); it < chunk.end(); it++)
        {
            SHUFFLE_RET pair = *it;
            mapReduceBase->Reduce(pair.first, pair.second);
        }
    }

    pthread_exit(0);
}


/**
 * @brief The library funciton the emits a <k3,v3> pair into the container of the current thread.
 * @param k3Base A pointer to the k3base.
 * @param v3Base A pointer to the v3base.
 */
void Emit3(k3Base *k3, v3Base *v3)
{
    pthread_t threadID = pthread_self();
    ExecReduceThread *t = (ExecReduceThread *) pthreadToThreadObject.at(threadID);
    OUT_ITEMS_VEC *container = &(t->container);

    // Lock the container before the addition
    int res = pthread_mutex_lock(&(t->containerMutex));
    if (res < 0)
    {
        exitWithError("pthread_mutex_lock");
    }

    container->push_back(make_pair(k3, v3));

    res = pthread_mutex_unlock(&(t->containerMutex));
    if (res < 0)
    {
        exitWithError("pthread_mutex_unlock");
    }
}

/**
 * @brief Prints the received content to the log file.
 * @param content The content to append to the log file.
 */
void printToLog(string content)
{
    int res = pthread_mutex_lock(&logFile_mutex);
    if (res < 0)
    {
        exitWithError("pthread_mutex_lock");
    }

    if (logFile.is_open())
    {
        logFile << content << endl;
    }
    else
    {
        exitWithError("open");
    }

    res = pthread_mutex_unlock(&logFile_mutex);
    if (res < 0)
    {
        exitWithError("pthread_mutex_unlock");
    }
}


/**
 * @brief Releases all the resources that were obtained by the framework.
 */
void releaseResources(bool autoDeleteV2K2)
{
    int res = pthread_mutex_destroy(&k1v1_mutex);
    if (res < 0) exitWithError("pthread_mutex_destroy");
    res = pthread_mutex_destroy(&unfinishedThreads_mutex);
    if (res < 0) exitWithError("pthread_mutex_destroy");
    res = pthread_mutex_destroy(&shuffle_mutex);
    if (res < 0) exitWithError("pthread_mutex_destroy");
    res = pthread_mutex_destroy(&pthreadToContainer_mutex);
    if (res < 0) exitWithError("pthread_mutex_destroy");
    res = pthread_mutex_destroy(&logFile_mutex);
    if (res < 0) exitWithError("pthread_mutex_destroy");

    if (autoDeleteV2K2)
    {
        for (auto it = execMapThreads.begin(); it < execMapThreads.end(); it++)
        {
            ExecMapThread *thread = *it;
            delete thread;
        }
    }

    for (auto it = execReduceThreads.begin(); it < execReduceThreads.end(); it++)
    {
        ExecReduceThread *thread = *it;
        delete thread;
    }

    res = sem_destroy(&shuffleSem);
    if (res < 0) exitWithError("sem_destroy");

    logFile.close();
}