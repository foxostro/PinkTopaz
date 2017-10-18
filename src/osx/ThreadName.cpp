//
//  ThreadName.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/17/17.
//
//

#include "ThreadName.hpp"
#include "Exception.hpp"
#include <pthread.h>
#include <errno.h>

void setNameForCurrentThread(const std::string &name)
{
    int r = pthread_setname_np(name.c_str());
    switch (r) {
        case 0:
            // success
            break;

        case ERANGE:
            throw Exception("setNameForCurrentThread: The length of the string "
                            "specified pointed to by name exceeds the allowed "
                            "limit. name=\"%s\"", name.c_str());

        default:
            throw Exception("setNameForCurrentThread: unknown error %d", r);
    }
}
