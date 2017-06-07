//
// Created by avishadler on 5/28/17.
//

#include "CacheFS.h"
#include "FileDesc.h"
#include "CacheBlock.h"
#include "LRUCache.h"
#include "LFUCache.h"
#include "FBRCache.h"


#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <malloc.h>


#define SUCCESS 0
#define FAILURE -1
#define MAX_PATH 5000

using namespace std;
cache_algo_t state;
struct stat fi;
size_t blksize;
bool destroy = true;

map<int, FileDesc> filesMap;
Cache *cache;
int hits;
int misses;


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
        return FAILURE;
    }
    if (blocks_num <= 0) {
        return FAILURE;
    }
    hits = 0;
    misses = 0;
    if (cache_algo == LRU) {
        cache =  new LRUCache(blocks_num);
        state = LRU;
    } else if (cache_algo == LFU) {
        cache = new LFUCache(blocks_num);
        state = LFU;
    } else if (cache_algo == FBR) {
        if(f_old >= 1 || f_old <= 0 || f_new >= 1 || f_new <= 0 || f_old+f_new > 1){
            return FAILURE;
        }
        cache = new FBRCache(blocks_num, f_new,f_old);
        state = FBR;
    } else {
        return -1;
    }
    destroy = false;
    return SUCCESS;
}

int CacheFS_destroy() {
    delete cache;
    filesMap.clear();
    destroy = true;
    return SUCCESS;
}


std::ifstream::pos_type filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
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
    char *res = realpath(pathname, NULL);
    if (!res) {
        return FAILURE;
    }
    string fullPath = string(res);
    string path = string(fullPath);
    if(path.find("/tmp")== string::npos){
        return FAILURE;
    }
    int fd = open(pathname, O_RDONLY | O_DIRECT | O_SYNC);
    if (fd < 0) {
        return FAILURE;
    }

    auto it = filesMap.find(fd);
    if (it == filesMap.end()) {
        FileDesc file = FileDesc(fd, path);
        filesMap[fd] = file;
        cache->blocks[fullPath];
    }
    free(res);
    return fd;
}

int CacheFS_pread(int file_id, void *buf, size_t count, off_t offset) {
    auto it = filesMap.find(file_id);
    if (it == filesMap.end()) {
        return FAILURE;
    }

    FileDesc file = it->second;
    string path = file.getPath();
    int b_read = 0;
    double blockSize = (double) blksize;
    long double start = floor(offset / blockSize);
    int end = (int) ceil((offset + count)/ blockSize);
    off_t where = lseek(file_id, (size_t) start, SEEK_SET);
    if (where < 0) {
        return FAILURE;
    }
    for (int i = (int) start; i < end; i++) {
        char* curbuf;
        // Check if the block of the file exists in the cache
        CacheBlock *block = cache->readBlock(path, i);
        int endOfRead;
        if (block != nullptr) {
            hits++;
            curbuf = block->buf;
        } else {
            // The block isn't saved in the cache
            curbuf = (char*)aligned_alloc(blksize,blksize);
            int numOfBytes = (int) pread(file_id, curbuf, (size_t) blksize, i*(size_t)blksize);
            if (numOfBytes < 0) {
                return FAILURE;
            }
            if (numOfBytes == 0) continue;
            misses++;
            CacheBlock *blockToAdd = new CacheBlock(path, i, curbuf, (int) blksize);
            cache->cacheBlock(blockToAdd);
        }
        memcpy(buf, curbuf + (offset%blksize) , count);
    }
    int sizeOfFile = (int) filesize(path.c_str());
    if(offset+count > sizeOfFile){
        b_read = sizeOfFile- (int) offset;
        if(b_read < 0){
            b_read = 0;
        }
    }
    else{
        b_read = (int) count;
    }


    return b_read;
}

int CacheFS_close(int file_id) {
    return close(file_id);
}

int CacheFS_print_cache (const char *log_path){
    ofstream file;
    file.open(log_path, std::ofstream::out | std::ofstream::app);
    if(file.fail()){
        return FAILURE;
    }
    file<<cache->printCache();
    file.close();
    return SUCCESS;
}

int CacheFS_print_stat (const char *log_path){
    ofstream file;
    file.open(log_path, std::ofstream::out | std::ofstream::app);
    if(file.fail()){
        return FAILURE;
    }
    cout << "Hits number: " << to_string(hits) << "." << endl <<"Misses number: " << to_string(misses) << "." << endl;
    file << "Hits number: " << to_string(hits) << "." << endl <<"Misses number: " << to_string(misses) << "." << endl;
    file.close();
    return SUCCESS;
}
