//
// Created by Gilad Lumbroso on 07/05/2017.
//

#define SUCCESS 0
#define FAILURE 1

#include <stdlib.h>
#include <string>
#include <cstdlib>
#include <iostream>
#include "MapReduceClient.h"
#include "MapReduceFramework.h"
#include <vector>
#include <fstream>
#include <dirent.h>


using namespace std;


string search_str;
vector<string> paths;


class DirNameKey: public k1Base {

    string dirName;

    DirNameKey(string dn) {
        dirName = dn;
    }

    bool operator<(const k1Base &other) const override {
        const DirNameKey &otherDirName = (const DirNameKey &) other;
        return this->dirName < otherDirName.dirName;
    }
};

class SearchTermValue: public v1Base {

    string searchTerm;

    SearchTermValue(string st) {
        searchTerm = st;
    }
};

class FileNameKey: public k2Base, k3Base {

    string fileName;

    FileNameKey(string fn) {
        fileName = fn;
    }

    bool operator<(const k2Base &other) const override {
        const FileNameKey &otherFileName = (const FileNameKey &) other;
        return this->fileName < otherFileName.fileName;
    }
};


class SingleCountValue: public v2Base {

    int count;

    SingleCountValue() {
        count = 1;
    }
};


class FileCountValue: public v3Base {

    int count;

    FileCountValue(int c) {
        count = c;
    }
};


class MapReduce: public MapReduceBase {

    MapReduce() {}

    void Map(const k1Base *const key, const v1Base *const val) const override {

    }

    void Reduce(const k2Base *const key, const V2_VEC &vals) const override {

    }

};


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