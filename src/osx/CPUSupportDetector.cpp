//
//  CPUSupportDetector.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 9/16/17.
//
//

#include "CPUSupportDetector.hpp"
#include <x86intrin.h>

bool isAVX2Supported()
{
    return __builtin_cpu_supports("avx2");
}
