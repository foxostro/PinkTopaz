//
//  Stopwatch.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/13/17.
//
//

#include "Stopwatch.hpp"
#include <mach/mach_time.h>

Stopwatch::Stopwatch()
{
    mach_timebase_info_data_t timebaseInfo;
    mach_timebase_info(&timebaseInfo);
    _absToNanos = (double)timebaseInfo.numer / timebaseInfo.denom;
    _nanosToAbs = (double)timebaseInfo.denom / timebaseInfo.numer;
}

uint64_t Stopwatch::getCurrentTimeInNanos() const
{
    return (uint64_t)(mach_absolute_time() * _absToNanos);
}

void Stopwatch::waitUntil(uint64_t wakeUpTime) const
{
    mach_wait_until((uint64_t)(wakeUpTime * _nanosToAbs));
}
