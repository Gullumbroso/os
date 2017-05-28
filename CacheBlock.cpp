//
// Created by avishadler on 5/28/17.
//

#include "CacheBlock.h"

CacheBlock::CacheBlock(string filePath, int pos, void *buf, off_t offset) :
        filePath(filePath), pos(pos), buf(buf), offset(offset){

}

CacheBlock::~CacheBlock() {

}

bool CacheBlock::operator==(const CacheBlock &other) const {
    return this->pos == other.pos;
}
