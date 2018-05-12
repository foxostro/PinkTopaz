//
//  ThreadName.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/17/17.
//
//

#include "ThreadName.hpp"
#include "PosixException.hpp"
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
            throw PosixException("setNameForCurrentThread: The length of"
                                 " the string specified in `name' "
                                 "exceeds the allowed limit. "
                                 "name=\"{}\"", name);
            
        default:
            throw PosixException("setNameForCurrentThread: error {}", r);
    }
}
