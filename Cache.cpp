//
// Created by avishadler on 6/6/17.
//

#include "CacheBlock.h"
#include "Cache.h"


Cache::Cache() {

}

Cache::Cache(int bn) {
    blockNum = bn;
}

CacheBlock *Cache::findBlock(string path, int blockNum) {
    CacheBlock *blockToCheck;
    for(auto it = blocks[path].begin(); it<blocks[path].end();it++){
        blockToCheck = *it;
        if (blockToCheck->pos == blockNum){
            return blockToCheck;
        }
    }
    return nullptr;
}

Cache::~Cache() {
    blocks.clear();
}
