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
    Cache();
    virtual ~Cache();
    Cache(int blockNum);
    map<string, vector<CacheBlock *>> blocks;
    int blockNum;
    virtual void cacheBlock(CacheBlock *block);
    virtual CacheBlock *findBlock(string path, int blockNum);
    virtual CacheBlock *readBlock(string path, int blockNum) = 0;
    virtual string printCache() = 0;

};





#endif //UNTITLED1_ALGO_H
