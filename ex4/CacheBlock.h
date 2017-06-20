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
    char *buf;
    int numOfBytes;
    int references;


    CacheBlock();

    /**
     * the main constructor for the cacheBlock repr.
     * @return cacheBlock.
     */
    CacheBlock(string filePath, int pos, char *buf, int numOfBytes);

    /**
     * the destructor of the cacheBlock class
     */
    ~CacheBlock();

    /**
     * overide the == operator. comparison by id.
     * @param other - the other File.
     * @return true if the id is equal. false, otherwise.
     */
    bool operator ==(const CacheBlock &other) const;

};


#endif //UNTITLED1_CACHE_H
