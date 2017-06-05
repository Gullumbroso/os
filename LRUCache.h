//
// Created by gullumbroso on 6/5/17.
//

#ifndef UNTITLED1_LRUALGO_H
#define UNTITLED1_LRUALGO_H

#include "CacheBlock.h"
#include <string>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>

#include "Cache.h"



class LRUCache : public Cache {
public:
    vector<CacheBlock *> cacheQueue;
    LRUCache(int blockNum);
    void cacheBlock(CacheBlock &block) override;
    CacheBlock* readBlock(string path, int blockNum) override ;

};


#endif //UNTITLED1_LRUALGO_H
