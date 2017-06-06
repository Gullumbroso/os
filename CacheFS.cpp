//
// Created by avishadler on 5/28/17.
//

#include "CacheFS.h"
#include "File.h"
#include "CacheBlock.h"
#include "LRUCache.h"
#include "LFUCache.h"
#include "FBRCache.h"


#include <string>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
#include <iomanip>
#include <sstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include "Cache.h"
#include <stdio.h>


#define SUCCESS 0
#define FAILURE -1
#define MAX_PATH 5000

using namespace std;
cache_algo_t state;
struct stat fi;
size_t blksize;
bool destroy = true;

map<int, File> filesMap;
Cache cache;


/**
 * @brief Prints an error message to the standard error with the name of the failing function.
 * @param failingMsg The name of the failing function.
 */
void exitWithError(string failingMsg) {
    cerr << "CacheFS Failure: " << failingMsg << endl;
    exit(FAILURE);
}

int CacheFS_init(int blocks_num, cache_algo_t cache_algo, double f_old, double f_new) {
    stat("/tmp", &fi);
    blksize = (size_t) fi.st_blksize;

    if (!destroy) {
        exitWithError("Can not do init if ChacheFS was not destroyed before");
    } else {
        destroy = false;
    }
    if (blocks_num <= 0) {
        exitWithError("invalid number of blocks");
    }
    if (cache_algo == LRU) {
        cache = LRUCache(blocks_num);
        state = LRU;
    } else if (cache_algo == LFU) {
        cache = LFUCache(blocks_num);
        state = LFU;
    } else if (cache_algo == FBR) {
        if(f_old >= 1 || f_old <= 0 || f_new >= 1 || f_new <= 0 || f_old+f_new > 1){
            exitWithError("invalid f_old or f_new input");
        }
        cache = FBRCache(blocks_num, f_new,f_old);
        state = FBR;
    } else {
        return -1;
    }
}

int CacheFS_destroy() {
    destroy = true;
    return 0;
}

/**
 File open operation.
 Receives a path for a file, opens it, and returns an id
 for accessing the file later

    pathname - the path to the file that will be opened

 Returned value:
    In case of success:
		Non negative value represents the id of the file.
		This may be the file descriptor, or any id number that you wish to create.
		This id will be used later to read from the file and to close it.

 	In case of failure:
		Negative number.
		A failure will occur if:
			1. System call or library function fails (e.g. open).
			2. Invalid pathname. Pay attention that we support only files under
			   "/tmp" due to the use of NFS in the Aquarium.
 */
int CacheFS_open(const char *pathname) {
    char *fullPath;
    char *res = realpath(pathname, fullPath);
    if (!res) {
        exitWithError("can not get full path");
    }
    string path = string(pathname);
    if(!(path.find("/tmp"))){
        exitWithError("cannot open file");
    }
    int fd = open(pathname, O_RDONLY | O_DIRECT | O_SYNC);
    if (fd < 0) {
        exitWithError("open file is not valid.");
    }

    auto it = filesMap.find(fd);
    if (it == filesMap.end()) {
        File file = File(fd, string(fullPath));
        filesMap[fd] = file;
        char *buf = new char[MAX_PATH];
        cache.blocks[fullPath];
    }
    return 0;
}

int CacheFS_pread(int file_id, void *buf, size_t count, off_t offset) {
    auto it = filesMap.find(file_id);
    if (it == filesMap.end()) {
        exitWithError("trying to read a file that is not open.");
    }
    char *currentPointer = static_cast<char*>(buf);
    File file = it->second;
    string path = file.getPath();
    int b_read = 0;
    int start = (int) floor(offset / blksize);
    int end = (int) floor((offset + count) / blksize);
    off_t where = lseek(file_id, start, SEEK_SET);
    if (where < 0) {
        exitWithError("can't read from file.");
    }

    vector<CacheBlock> blocks = cache.blocks[path];
    for (int i = start; i < end; i++) {
        // Check if the block of the file exists in the cache
        CacheBlock *block = cache.readBlock(path, i);
        if (block != nullptr) {
            b_read += block->numOfBytes;
            *currentPointer = *block->buf;
            currentPointer += block->numOfBytes;
        } else {
            // The block isn't saved in the cache
            int numOfBytes = (int) read(file_id, currentPointer, blksize);
            if (numOfBytes < 0) {
                exitWithError("Couldn't read from a file.");
            }
            *currentPointer = *(static_cast<char*>(buf));
            currentPointer += numOfBytes;
            b_read += numOfBytes;
            CacheBlock blockToAdd = CacheBlock(path, i, currentPointer, numOfBytes);
            cache.cacheBlock(blockToAdd);
        }

    }

    return b_read;
}


int CacheFS_close(int file_id) {
    return 0;
}