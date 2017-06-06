//
// Created by avishadler on 6/6/17.
//

#ifndef UNTITLED1_FBRCACHE_H
#define UNTITLED1_FBRCACHE_H

#include "CacheBlock.h"
#include "Cache.h"
#include <string>
#include <set>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
#include <math.h>

using namespace std;

class FBRCache : public Cache{
public:
    double newBlocks;
    double oldBlocks;
    vector<CacheBlock *> cacheQueue;
    FBRCache(int blockNum, double newBlocks, double oldBlocks);
    void cacheBlock(CacheBlock *block) override;
    CacheBlock* readBlock(string path, int blockNum) override;
    string printCache() override ;
    ~FBRCache();
};


#endif //UNTITLED1_FBRCACHE_H
