//
// Created by avishadler on 6/6/17.
//

#include "FBRCache.h"

FBRCache::FBRCache(int blockNum, double newBlocks, double oldBlocks) : blockNum(blockNum), newBlocks(newBlocks),
oldBlocks(oldBlocks){

}

void FBRCache::cacheBlock(CacheBlock &block) {
    if (cacheQueue.size() >= blockNum) {
        CacheBlock *toErase = cacheQueue.back();
        string fp = toErase->filePath;
        int old = (int) floor(cacheQueue.size()/this->oldBlocks);
        auto it = cacheQueue.end() - old;
        CacheBlock* chosenBlock = *it;

        int minReferences;
        minReferences = chosenBlock->references;
        CacheBlock* blockToCheck;
        auto minIt = it;
        while(it != cacheQueue.end()){
            blockToCheck = *it;
            if(blockToCheck->references < minReferences){
                minReferences = blockToCheck->references;
                chosenBlock = blockToCheck;
                minIt = it;
            }
        }
        int id = chosenBlock->pos;
        cacheQueue.erase(minIt);
        auto it2 = find(blocks[fp].begin(), blocks[fp].end(), id);
        blocks[fp].erase(it2);
    }
    cacheQueue.insert(cacheQueue.begin(), &block);
    blocks[block.filePath].push_back(block);
}

CacheBlock *FBRCache::readBlock(string path, int blockNum) {
    CacheBlock *block = Cache::readBlock(path, blockNum);
    if (block != nullptr) {
        auto it = find(cacheQueue.begin(), cacheQueue.end(), block);
        int endOfNew = (int) ceil(cacheQueue.size()/this->newBlocks);
        CacheBlock * toAdd = *it;
        if(!(it>=cacheQueue.begin() && it < cacheQueue.begin()+endOfNew)){
            toAdd->references++;
        }
        cacheQueue.erase(it);
        cacheQueue.insert(cacheQueue.begin(), toAdd);
    }
    return block;
}
