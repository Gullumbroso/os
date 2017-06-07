//
// Created by avishadler on 6/6/17.
//

#include "FBRCache.h"

FBRCache::FBRCache(int blockNum, double newBlocks, double oldBlocks) : Cache(blockNum), newBlocks(newBlocks),
oldBlocks(oldBlocks){

}

void FBRCache::cacheBlock(CacheBlock *block) {
    if (cacheQueue.size() >= (unsigned int)blockNum) {
        int old = (int) ceil(cacheQueue.size()*this->oldBlocks);
        auto it = cacheQueue.end() - old;
        CacheBlock* chosenBlock = *it;

        int minReferences;
        minReferences = chosenBlock->references;
        CacheBlock* blockToCheck;
        auto minIt = it;
        while(it != cacheQueue.end()){
            blockToCheck = *it;
            if(blockToCheck->references <= minReferences){
                minReferences = blockToCheck->references;
                chosenBlock = blockToCheck;
                minIt = it;
            }
            it++;
        }
        int id = chosenBlock->pos;
        string fp = chosenBlock->filePath;
        cacheQueue.erase(minIt);

        CacheBlock * chosenToErase = Cache::findBlock(chosenBlock->filePath, id);
        auto it2 = find(blocks[fp].begin(), blocks[fp].end(), chosenToErase);
        blocks[chosenToErase->filePath].erase(it2);
    }
    cacheQueue.insert(cacheQueue.begin(), block);
    blocks[block->filePath].push_back(block);
}

CacheBlock *FBRCache::readBlock(string path, int blockNum) {
    CacheBlock *block = Cache::findBlock(path, blockNum);
    if (block != nullptr) {
        auto it = find(cacheQueue.begin(), cacheQueue.end(), block);
        int endOfNew = (int) ceil(cacheQueue.size()*this->newBlocks);
        CacheBlock * toAdd = *it;
        if(!(it>=cacheQueue.begin() && it < cacheQueue.begin()+endOfNew)){
            toAdd->references++;
        }
        cacheQueue.erase(it);
        cacheQueue.insert(cacheQueue.begin(), toAdd);
    }
    return block;
}

string FBRCache::printCache() {
    string finalOutput;
    for(auto it = cacheQueue.begin() ; it < cacheQueue.end();it++){
        CacheBlock * block = *it;
        finalOutput += block->filePath;
        finalOutput+=" ";
        finalOutput+= to_string(block->pos);
        finalOutput+="\n";
    }
    return finalOutput;
}

FBRCache::~FBRCache() {
    for (auto it = cacheQueue.begin(); it < cacheQueue.end(); it++) {
        CacheBlock *block = *it;
        delete block;
    }
}
