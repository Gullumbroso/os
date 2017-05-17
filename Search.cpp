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
IN_ITEMS_VEC inputItems;
OUT_ITEMS_VEC outputItems;

pthread_mutex_t printMutexUser = PTHREAD_MUTEX_INITIALIZER;
void safePrintUser(string msg) {
    pthread_mutex_lock(&printMutexUser);
    cout << msg << endl;
    pthread_mutex_unlock(&printMutexUser);
}


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
 * @brief Implements the k2Base class.
 */
class FileNameKey2 : public k2Base
{

public:
    string fileName;

    FileNameKey2(string fn)
    {
        fileName = fn;
    }

    bool operator<(const k2Base &other) const override
    {
        const FileNameKey2 &otherFileName = (const FileNameKey2 &) other;
        return this->fileName < otherFileName.fileName;
    }
};


/**
 * @brief Implements the k3Base class
 */
class FileNameKey3 : public k3Base
{

public:
    string fileName;

    FileNameKey3(string fn)
    {
        fileName = fn;
    }

    bool operator<(const k3Base &other) const override
    {
        const FileNameKey3 &otherFileName = (const FileNameKey3 &) other;
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
        safePrintUser("In Map func " + string(to_string(pthread_self())));

        DirNameKey *const dirName = (DirNameKey *const) key;
        SearchTermValue *const word = (SearchTermValue *const) val;

        vector<string> res = searchInPath(dirName->dirName, word->searchTerm);

        for (auto it = res.begin(); it < res.end(); it++)
        {
            safePrintUser("In the for loop of the Map func " + string(to_string(pthread_self())));

            string fileName = *it;
            FileNameKey2 *file = new FileNameKey2(fileName);
            Emit2(file, new SingleCountValue());
        }
    }


    void Reduce(const k2Base *const key, const V2_VEC &vals) const override
    {
        FileNameKey2 *const fileName = (FileNameKey2 *const) key;
        string fn = fileName->fileName;
        FileNameKey3 *const fileNameKey3 = new FileNameKey3(fn);
        FileCountValue *const count = new FileCountValue((int) vals.size());
        outputItems.push_back(pair<FileNameKey3 *, FileCountValue *> {fileNameKey3, count});
        Emit3(fileNameKey3, count);
    }
};


/**
 * @brief Releases all the resources that were obtained by the framework.
 */
void releaseResources()
{
    for (auto it = inputItems.begin(); it < inputItems.end(); it++)
    {
        auto pair = *it;
        delete pair.first;
        delete pair.second;
    }

    for (auto it = outputItems.begin(); it < outputItems.end(); it++)
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
void abort(string message)
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
    if ((dir = opendir(path_to_check.c_str())) != NULL)
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
        abort("Could not open directory");
    }

    return relevantFiles;
}


/**
 * @brief Prints the final output.
 * @param finalOutput The final output.
 */
void printResults(OUT_ITEMS_VEC finalOutput)
{
    for (auto it = finalOutput.begin(); it < finalOutput.end(); it++)
    {
        OUT_ITEM pair = *it;
        FileNameKey3 *fileNameKey3 = (FileNameKey3 *)pair.first;
        FileCountValue *countValue = (FileCountValue *)pair.second;
        for (int i = 0; i < countValue->count; i++)
        {
            cout << fileNameKey3->fileName;
        }
    }
}


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        abort("Wrong number of arguments.");
    }
    else
    {
        search_str = argv[1];
        paths.insert(paths.end(), argv + 2, argv + argc);
    }

    // prepare the IN_ITEMS_VEC for the runMapReduceFramework function
    for (auto it = paths.begin(); it < paths.end(); it++)
    {
        string path = *it;
        k1Base *k1 = new DirNameKey(path);
        v1Base *v1 = new SearchTermValue(search_str);
        pair<k1Base*, v1Base*> pair1 = {k1, v1};
        inputItems.push_back(pair1);
    }

    MapReduce mapReduce;

    OUT_ITEMS_VEC finalOutput = RunMapReduceFramework(mapReduce, inputItems, MULTI_THERAD_LEVEL,
                                                      true);

    printResults(finalOutput);

    releaseResources();

    return SUCCESS;
}