//
// Created by gullumbroso on 5/28/17.
//

#include "CacheFS.h"
#include "CacheBlock.h"

#include <string>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <iomanip>


int main ()
{
    map<string, vector<int>> cache;
    cache["Gilad"];
    cache["Gilad"].push_back(54);
    cache["Gilad"].push_back(1);
    cache["Gilad"].push_back(2);

    for (auto it = cache["Gilad"].begin(); it < cache["Gilad"].end(); it++)
    {
        cout << *it << endl;
    }
    return 0;
}