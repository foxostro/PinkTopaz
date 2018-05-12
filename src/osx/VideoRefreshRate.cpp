//
//  VideoRefreshRate.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/13/17.
//
//

#include "VideoRefreshRate.hpp"
#include "Exception.hpp"
#include <limits>
#include <ApplicationServices/ApplicationServices.h>

boost::optional<double> getVideoRefreshRate()
{
    // Get the lowest refresh rate across all displays. It's safe to refresh at
    // this rate even if the window moves between displays.
    
    constexpr size_t MAX_DISPLAYS = 32;
    double bestRefreshRate = std::numeric_limits<double>::infinity();
    
    CGDirectDisplayID displays[MAX_DISPLAYS];
    CGDisplayCount count;
    CGDisplayErr displayErr = CGGetOnlineDisplayList(MAX_DISPLAYS, displays, &count);
    
    if(displayErr == 0) {
        // Failed to get the online displays.
        return boost::none;
    }
    
    for (CGDisplayCount i = 0; i < count; ++i) {
        CGDisplayModeRef mode = CGDisplayCopyDisplayMode(displays[i]);
        if (mode) {
            double refreshRate = CGDisplayModeGetRefreshRate(mode);
            CGDisplayModeRelease(mode);
            
            if (refreshRate == 0.0) {
                refreshRate = 60.0;
            }
            
            if (refreshRate < bestRefreshRate) {
                bestRefreshRate = refreshRate;
            }
        }
    }
    
    return bestRefreshRate;
}
