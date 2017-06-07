//
// Created by gullumbroso on 6/5/17.
//

#include "LRUCache.h"


LRUCache::LRUCache(int blockNum) : Cache(blockNum) {
}

void LRUCache::cacheBlock(CacheBlock *block) {

    auto it = find(cacheQueue.begin(), cacheQueue.end(), block);
    if (it != cacheQueue.end()) {
        cout << "Error!" << endl;
    }

    if (cacheQueue.size() >= blockNum) {
        CacheBlock *toErase = cacheQueue.back();
        string fp = toErase->filePath;
        int id = toErase->pos;
        cacheQueue.pop_back();
        CacheBlock * chosenToErase = Cache::findBlock(fp, id);
        auto it2 = find(blocks[fp].begin(), blocks[fp].end(), chosenToErase);
        blocks[fp].erase(it2);
    }
    cacheQueue.insert(cacheQueue.begin(), block);
    blocks[block->filePath].push_back(block);
}


CacheBlock *LRUCache::readBlock(string path, int blockNum) {
    CacheBlock *block = Cache::findBlock(path, blockNum);
    if (block != nullptr) {
        auto it = find(cacheQueue.begin(), cacheQueue.end(), block);
        CacheBlock *toAdd = *it;
        cacheQueue.erase(it);
        cacheQueue.insert(cacheQueue.begin(), toAdd);
    }
    return block;
}

string LRUCache::printCache() {
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

LRUCache::~LRUCache() {

        for ( int i = 0; i < cacheQueue.size(); i++ )
        {
            delete cacheQueue[i];
        }
        cacheQueue.clear();
}