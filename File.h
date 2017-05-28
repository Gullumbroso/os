//
// Created by avishadler on 5/28/17.
//

#ifndef UNTITLED1_FILE_H
#define UNTITLED1_FILE_H

#include <string>

using namespace std;

class File {

public:
    /**
     * the main constructor for the file repr.
     * @param id - the id to set for the file.
     * @param name - the name to set for the file.
     * @return the File.
     */
    File(int id, string name);

    /**
     * the destructor of the File class
     */
    ~File();

    /**
     * gets the ID of the File
     * @return ID of the file.
     */
    int getId();

    /**
     * gets the name of the File
     * @return string name of the file.
     */
    string getName();

    /**
     * the number of times we called the file.
     * @return the number of times we called the file.
     */
    int getFrequency();

    /**
     * increment by 1 the number of times we called the file.
     */
    void incFrequency();

    /**
     * overide the == operator. comparison by id.
     * @param other - the other File.
     * @return true if the id is equal. false, otherwise.
     */
    bool File::operator ==(const File &other) const;

private:
    string path;

    int id;

    // the number of times we called the file.
    int frequency;


};


#endif //UNTITLED1_FILE_H
