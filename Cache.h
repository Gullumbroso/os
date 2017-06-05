//
// Created by gullumbroso on 6/5/17.
//

#ifndef UNTITLED1_ALGO_H
#define UNTITLED1_ALGO_H

#include <string>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>

class Cache {
public:
    map<string, vector<CacheBlock>> blocks;
    int blockNum;

    virtual void cacheBlock(CacheBlock &block) = 0;

    virtual CacheBlock *readBlock(string path, int blockNum);


};

CacheBlock *Cache::readBlock(string path, int blockNum) {
    auto it = find(blocks[path].begin(), blocks[path].end(), blockNum);
    if (it == blocks.end()) {
        return nullptr;
    }
    return &(*it);
}


#endif //UNTITLED1_ALGO_H
