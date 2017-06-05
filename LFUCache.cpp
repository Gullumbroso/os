//
// Created by gullumbroso on 6/5/17.
//

#include "LFUCache.h"

inline bool fncomp (CacheBlock *lcb, CacheBlock *rcb) {return lcb->references < rcb->references;}

LFUCache::LFUCache(int blockNum): blockNum(blockNum) {
    bool(*fn_pt)(CacheBlock *,CacheBlock *) = fncomp;
    std::set<CacheBlock *,bool(*)(CacheBlock *,CacheBlock *)> cq(fn_pt);
    cacheQueue = cq;
}

void LFUCache::cacheBlock(CacheBlock &block) {
    if (cacheQueue.size() >= blockNum) {
        auto it = cacheQueue.begin();
        CacheBlock *toErase = *it;
        string fp = toErase->filePath;
        int id = toErase->pos;
        cacheQueue.erase(it);
        auto it2 = find(blocks[fp].begin(), blocks[fp].end(), id);
        blocks[fp].erase(it2);
    }
    cacheQueue.insert(&block);
    blocks[block.filePath].push_back(block);
}

CacheBlock * LFUCache::readBlock(string path, int blockNum) {
    CacheBlock *block = Cache::readBlock(path, blockNum);
    if (block != nullptr) {
        auto it = find(cacheQueue.begin(), cacheQueue.end(), block);
        CacheBlock * b = *it;
        b->references++;
    }
    return block;
}
