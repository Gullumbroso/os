//
// Created by gullumbroso on 5/28/17.
//

#include "CacheFS.h"
#include "CacheBlock.h"


#include <iostream>
#include <fstream>

#include <iomanip>

int main () {
    ofstream file;
    file.open("/cs/usr/avishadler/safe/os/ex33/avishay.log", std::ofstream::out | std::ofstream::app);
    file<<"ZB";
    file.close();
    return 0;
}
