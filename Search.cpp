//
// Created by Gilad Lumbroso on 07/05/2017.
//

#define SUCCESS 0
#define FAILURE 1
#define MULTI_THERAD_LEVEL 4

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

vector<string> searchInPath(string path_to_check , string key_word);

class DirNameKey: public k1Base {

    public:
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
public:
    string searchTerm;

    SearchTermValue(string st) {
        searchTerm = st;
    }
};

class FileNameKey: public k2Base, public k3Base {

public:
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
public:

    int count;

    SingleCountValue() {
        count = 1;
    }
};


class FileCountValue: public v3Base {
public:

    int count;

    FileCountValue(int c) {
        count = c;
    }
};


class MapReduce: public MapReduceBase {

    void Map(const k1Base *const key, const v1Base *const val) const override {
        DirNameKey *const dirName = (DirNameKey *const) key;
        SearchTermValue *const word = (SearchTermValue *const) val;

        vector<string> res = searchInPath(dirName->dirName,word->searchTerm);
        for(auto it = res.begin(); it<res.end() ; it++){
            FileNameKey *file = new FileNameKey(*it);
            Emit2(file,new SingleCountValue());
        }
    }


    void Reduce(const k2Base *const key, const V2_VEC &vals) const override {
        FileNameKey *const fileName = (FileNameKey *const) key;
        FileCountValue *const count = new FileCountValue((int) vals.size());
        Emit3(fileName, count);
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
        paths.insert(paths.end(), argv + 2, argv+argc);
    }

    // prepare the IN_ITEMS_VEC for the runMapReduceFramework function
    IN_ITEMS_VEC k1v1Container;
    for (auto it = paths.begin(); it < paths.end(); it++)
    {
        k1v1Container.push_back(pair(*it, search_str));
    }

    MapReduce mapReduce;

    RunMapReduceFramework(mapReduce, k1v1Container, MULTI_THERAD_LEVEL, true);

    return 0;
}