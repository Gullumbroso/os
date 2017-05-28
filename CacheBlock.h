//
// Created by avishadler on 5/28/17.
//

#ifndef UNTITLED1_CACHE_H
#define UNTITLED1_CACHE_H
#include <string>

using namespace std;

class CacheBlock {

public:

    string filePath;
    int pos;
    void *buf;
    off_t offset;


    /**
     * the main constructor for the cacheBlock repr.
     * @return cacheBlock.
     */
    CacheBlock(string filePath, int pos, void *buf, off_t offset);

    /**
     * the destructor of the cacheBlock class
     */
    ~CacheBlock();
};


#endif //UNTITLED1_CACHE_H
