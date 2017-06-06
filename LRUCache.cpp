//
// Created by gullumbroso on 6/5/17.
//

#include "LRUCache.h"


LRUCache::LRUCache(int blockNum) : blockNum(blockNum) {
}

void LRUCache::cacheBlock(CacheBlock &block) {
    if (cacheQueue.size() >= blockNum) {
        CacheBlock *toErase = cacheQueue.back();
        string fp = toErase->filePath;
        int id = toErase->pos;
        cacheQueue.pop_back();
        auto it = find(blocks[fp].begin(), blocks[fp].end(), id);
        blocks[fp].erase(it);
    }
    cacheQueue.insert(cacheQueue.begin(), &block);
    blocks[block.filePath].push_back(block);
}


CacheBlock *LRUCache::readBlock(string path, int blockNum) {
    CacheBlock *block = Cache::readBlock(path, blockNum);
    if (block != nullptr) {
        auto it = find(cacheQueue.begin(), cacheQueue.end(), block);
        CacheBlock *toAdd = *it;
        cacheQueue.erase(it);
        cacheQueue.insert(cacheQueue.begin(), toAdd);
    }
    return block;
}
