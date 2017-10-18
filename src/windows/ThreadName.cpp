//
//  ThreadName.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/17/17.
//
//

#include "ThreadName.hpp"
#include <windows.h>

void setNameForCurrentThread(const std::string &name)
{
	// -1 specifies the current thread
    SetThreadName(-1, name.c_str());
}
