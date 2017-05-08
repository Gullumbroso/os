//
// Created by Gilad Lumbroso on 07/05/2017.
//

#define SUCCESS 0
#define FAILURE 1

#include <stdlib.h>
#include <string>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <fstream>
#include <dirent.h>


using namespace std;


string search_str;
vector<string> paths;

void exitWithError(string message)
{
    cerr << message << endl;
    exit(FAILURE);
}


vector<string> searchInPath(string path_to_check , string key_word){
    vector<string> relevantFiles;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir ("/cs/usr/avishadler/Desktop")) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            string name = ent->d_name;
            if (name.find(key_word) != std::string::npos) {
                relevantFiles.push_back(ent->d_name);
            }
        }
        closedir (dir);
    } else {
        /* could not open directory */
    }
    return relevantFiles;
}




int main(int argc, char *argv[]) {
    if (argc < 2)
    {
        exitWithError("Wrong number of arguments.");
    }
    else
    {
        search_str = argv[1];
        paths.insert(paths.end(), argv+2, argv+argc);
    }


    return 0;
}