//
// Created by gullumbroso on 5/9/17.
//

#ifndef PROJECT_EXECMAP_H
#define PROJECT_EXECMAP_H

#include "Thread.h"

/**
 * @brief A class that represents an ExecMap thread.
 */
class ExecMapThread: public Thread {
public:

    SHUFFLE_VEC container;

    // if true, deletes all the k2v2Base pointers of the container in the destructor. Depends on
    // the autoDeleteV2K2 boolean variable supplied by the user.
    bool deleteElements;


    ExecMapThread(bool autoDeleteV2K2);
    ~ExecMapThread();
};


#endif //PROJECT_EXECMAP_H
