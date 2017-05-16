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
vector<string> searchInPath(string path_to_check, string key_word);
IN_ITEMS_VEC k1v1Container;
OUT_ITEMS_VEC k3v3Container;


/**
 * @brief Implements the k1Base class
 */
class DirNameKey : public k1Base
{

public:
    string dirName;

    DirNameKey(string dn)
    {
        dirName = dn;
    }

    bool operator<(const k1Base &other) const override
    {
        const DirNameKey &otherDirName = (const DirNameKey &) other;
        return this->dirName < otherDirName.dirName;
    }
};


/**
 * @brief Implements the v1Base class
 */
class SearchTermValue : public v1Base
{
public:
    string searchTerm;

    SearchTermValue(string st)
    {
        searchTerm = st;
    }
};


/**
 * @brief Implements the k2Base and the k3Base classes
 */
class FileNameKey : public k2Base, public k3Base
{

public:
    string fileName;

    FileNameKey(string fn)
    {
        fileName = fn;
    }

    bool operator<(const k2Base &other) const override
    {
        const FileNameKey &otherFileName = (const FileNameKey &) other;
        return this->fileName < otherFileName.fileName;
    }
};


/**
 * @brief Implements the v2Base class
 */
class SingleCountValue : public v2Base
{
public:

    int count;

    SingleCountValue()
    {
        count = 1;
    }
};


/**
 * @brief Implements the v3Base class
 */
class FileCountValue : public v3Base
{
public:

    int count;

    FileCountValue(int c)
    {
        count = c;
    }
};


/**
 * @brief Contains the map and the reduce functions.
 */
class MapReduce : public MapReduceBase
{

    void Map(const k1Base *const key, const v1Base *const val) const override
    {
        DirNameKey *const dirName = (DirNameKey *const) key;
        SearchTermValue *const word = (SearchTermValue *const) val;

        vector<string> res = searchInPath(dirName->dirName, word->searchTerm);
        for (auto it = res.begin(); it < res.end(); it++)
        {
            FileNameKey *file = new FileNameKey(*it);
            Emit2(file, new SingleCountValue());
        }
    }


    void Reduce(const k2Base *const key, const V2_VEC &vals) const override
    {
        FileNameKey *const fileName = (FileNameKey *const) key;
        FileCountValue *const count = new FileCountValue((int) vals.size());
        k3v3Container.push_back(pair(fileName, count));
        Emit3(fileName, count);
    }
};


/**
 * @brief Releases all the resources that were obtained by the framework.
 */
void releaseResources()
{
    for (auto it = k1v1Container.begin(); it < k1v1Container.end(); it++)
    {
        auto pair = *it;
        delete pair.first;
        delete pair.second;
    }

    for (auto it = k3v3Container.begin(); it < k3v3Container.end(); it++)
    {
        auto pair = *it;
        delete pair.first;
        delete pair.second;
    }
}


/**
 * @brief Prints an error message to the standard error with the name of the failing function.
 * @param failingFunc The name of the failing function.
 */
void exitWithError(string message)
{
    cerr << message << endl;
    releaseResources();
    exit(FAILURE);
}


/**
 * @brief Searches for files in the received path.
 * @param path_to_check The path to search in.
 * @param key_word The key word to search.
 * @return A vector of the files with filenames that contain the key word.
 */
vector<string> searchInPath(string path_to_check, string key_word)
{
    vector<string> relevantFiles;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir("/cs/usr/avishadler/Desktop")) != NULL)
    {
        /* print all the files and directories within directory */
        while ((ent = readdir(dir)) != NULL)
        {
            string name = ent->d_name;
            if (name.find(key_word) != std::string::npos)
            {
                relevantFiles.push_back(ent->d_name);
            }
        }
        closedir(dir);
    }
    else
    {
        /* could not open directory */
    }
    return relevantFiles;
}


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        exitWithError("Wrong number of arguments.");
    }
    else
    {
        search_str = argv[1];
        paths.insert(paths.end(), argv + 2, argv + argc);
    }

    // prepare the IN_ITEMS_VEC for the runMapReduceFramework function
    for (auto it = paths.begin(); it < paths.end(); it++)
    {
        k1v1Container.push_back(pair(*it, search_str));
    }

    MapReduce mapReduce;

    OUT_ITEMS_VEC finalOutput = RunMapReduceFramework(mapReduce, k1v1Container, MULTI_THERAD_LEVEL,
                                                      true);

    releaseResources();

    // TODO: Print final output?

    return SUCCESS;
}