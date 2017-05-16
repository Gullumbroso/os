//
// Created by gullumbroso on 5/9/17.
//

#include "ExecMapThread.h"

ExecMapThread::ExecMapThread(bool autoDeleteV2K2)
{
    deleteElements = autoDeleteV2K2;
}

ExecMapThread::~ExecMapThread()
{
    if (deleteElements)
    {
        for (auto it = container.begin(); it < container.end(); it++)
        {
            SHUFFLE_ITEM item = *it;
            delete item.first;
            delete item.second;
        }
    }
}


