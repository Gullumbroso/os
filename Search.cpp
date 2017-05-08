//
// Created by Gilad Lumbroso on 07/05/2017.
//

#define SUCCESS 0
#define FAILURE 1

#include <stdlib.h>
#include <string>
#include <cstdlib>
#include <iostream>

using namespace std;

void exitWithError(string message)
{
    cerr << message << endl;
    exit(FAILURE);
}


int main(int argc, char *argv[]) {
    if (argc < 2)
    {
        exitWithError("Wrong number of arguments.");
    }
    else
    {

    }
}