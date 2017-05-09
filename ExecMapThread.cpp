//
// Created by gullumbroso on 5/9/17.
//

#include "ExecMapThread.h"

ExecMapThread::ExecMapThread() {
    containerMutex = PTHREAD_MUTEX_INITIALIZER;
}

ExecMapThread::~ExecMapThread() {
    pthread_mutex_destroy(&containerMutex);
}


