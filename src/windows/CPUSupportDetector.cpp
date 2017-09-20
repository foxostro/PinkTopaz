//
//  CPUSupportDetector.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 9/16/17.
//
//

#include "CPUSupportDetector.hpp"
#include <intrin.h>

bool isAVX2Supported()
{
    // See also <https://stackoverflow.com/a/22521619>

    bool avxSupported = false;

    int cpuInfo[4];
    __cpuid(cpuInfo, 1);

    bool osUsesXSAVE_XRSTORE = cpuInfo[2] & (1 << 27) || false;
    bool cpuAVXSuport = cpuInfo[2] & (1 << 28) || false;

    if (osUsesXSAVE_XRSTORE && cpuAVXSuport) {
        unsigned long long xcrFeatureMask = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
        avxSupported = (xcrFeatureMask & 0x6) == 0x6;
    }

    return avxSupported;
}
