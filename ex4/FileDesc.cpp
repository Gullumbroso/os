//
// Created by avishadler on 5/28/17.
//

#include "FileDesc.h"

/**
 * the main constructor for the file repr.
 * @param id - the id to set for the file.
 * @param name - the name to set for the file.
 * @return the File.
 */

FileDesc::FileDesc(int id, string name) : id(id) , path(name) {
    incFrequency();
}

/**
 * the destructor of the File class
 */
FileDesc::~FileDesc() {

}


/**
 * gets the ID of the File
 * @return ID of the file.
 */
int FileDesc::getId() {
    return this->id;
}

/**
 * the number of times we called the file.
 * @return the number of times we called the file.
 */
int FileDesc::getFrequency() {
    return this->frequency;
}

/**
 * increment by 1 the number of times we called the file.
 */
void FileDesc::incFrequency() {
    this->frequency++;

}
/**
 * overide the == operator. comparison by path.
 * @param other - the other File.
 * @return true if the id is equal. false, otherwise.
 */
bool FileDesc::operator==(const FileDesc &other) const {
    return this->path == other.path;
}

/**
 * gets the name of the File
 * @return string name of the file.
 */
string FileDesc::getPath() {
    return this->path;
}

FileDesc::FileDesc() {

}



