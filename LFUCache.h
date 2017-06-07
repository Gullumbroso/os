//
// Created by gullumbroso on 6/5/17.
//

#ifndef UNTITLED1_LFU_H
#define UNTITLED1_LFU_H

#include "CacheBlock.h"
#include <string>
#include <set>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>

#include "Cache.h"

using namespace std;


class LFUCache: public Cache {

public:
    multiset<CacheBlock *,bool(*)(CacheBlock *,CacheBlock *)> cacheQueue;
    LFUCache(int blockNum);
    void cacheBlock(CacheBlock *block) override;
    CacheBlock *readBlock(string path, int blockNum) override;
    string printCache() override ;
    ~LFUCache();
};


#endif //UNTITLED1_LFU_H
