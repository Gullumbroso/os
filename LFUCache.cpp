//
// Created by gullumbroso on 6/5/17.
//

#include "LFUCache.h"

inline bool fncomp (CacheBlock *lcb, CacheBlock *rcb) {return lcb->references < rcb->references;}

LFUCache::LFUCache(int blockNum): Cache(blockNum) {
    bool(*fn_pt)(CacheBlock *,CacheBlock *) = fncomp;
    std::set<CacheBlock *,bool(*)(CacheBlock *,CacheBlock *)> cq(fn_pt);
    cacheQueue = cq;
}

void LFUCache::cacheBlock(CacheBlock *block) {
    if (cacheQueue.size() >= blockNum) {
        auto it = cacheQueue.begin();
        CacheBlock *toErase = *it;
        string fp = toErase->filePath;
        int id = toErase->pos;
        cacheQueue.erase(it);
        CacheBlock * chosenToErase = Cache::findBlock(fp, id);
        auto it2 = find(blocks[fp].begin(), blocks[fp].end(), chosenToErase);
        blocks[fp].erase(it2);
    }
    cacheQueue.insert(block);
    blocks[block->filePath].push_back(block);
}

CacheBlock * LFUCache::readBlock(string path, int blockNum) {
    CacheBlock *block = Cache::findBlock(path, blockNum);
    if (block != nullptr) {
        auto it = find(cacheQueue.begin(), cacheQueue.end(), block);
        CacheBlock * b = *it;
        b->references++;
    }
    return block;
}

string LFUCache::printCache() {
        string finalOutput;
        for(auto it = cacheQueue.rbegin() ; it != cacheQueue.rend();it++){
            CacheBlock * block = *it;
            finalOutput += block->filePath;
            finalOutput+=" ";
            finalOutput+= block->pos;
            finalOutput+="\n";
        }
        return finalOutput;
}

LFUCache::~LFUCache() {
    for (auto it = cacheQueue.rbegin(); it != cacheQueue.rend(); it++) {
        CacheBlock *block = *it;
        delete block;
    }
}
