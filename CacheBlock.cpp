//
// Created by avishadler on 5/28/17.
//

#include "CacheBlock.h"

CacheBlock::CacheBlock() {
}

CacheBlock::CacheBlock(string filePath, int pos, char *buf, int numOfBytes) :
        filePath(filePath), pos(pos), buf(buf), numOfBytes(numOfBytes) {
    references = 1;
}

CacheBlock::~CacheBlock() {

}

bool CacheBlock::operator==(const CacheBlock &other) const {
    return (this->pos == other.pos) && (this->filePath == other.filePath);
}
