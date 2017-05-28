//
// Created by avishadler on 5/28/17.
//

#include "CacheFS.h";
#include <string>;




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

int CacheFS_init(int blocks_num, cache_algo_t cache_algo, double f_old, double f_new) {
    if (!destroy){
        return -1;
    } else {
        destroy = false;
    }

    if(blocks_num <= 0){
        return -1;
    }
    else if(cache_algo == LRU){

    } else if(cache_algo == LFU){

    } else if(cache_algo == FBR){

    }


    else{
        return -1;
    }
}

int CacheFS_destroy() {
    destroy = true;
    return 0;
}

int CacheFS_open(const char *pathname) {
    return 0;
}

int CacheFS_close(int file_id) {
    return 0;
}
