//
// Created by avishadler on 5/28/17.
//

#include "CacheBlock.h"

CacheBlock::CacheBlock(string filePath, int pos, void *buf, off_t offset) :
        filePath(filePath), pos(pos), buf(buf), offset(offset){

}

CacheBlock::~CacheBlock() {

}
