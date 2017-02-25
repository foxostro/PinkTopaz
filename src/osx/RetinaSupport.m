//
//  RetinaSupport.m
//  PinkTopaz
//
//  Created by Andrew Fox on 7/1/16.
//
//

#import <Cocoa/Cocoa.h>
#import "RetinaSupport.h"
#import "SDL_syswm.h"
    
float windowScaleFactor(SDL_Window *window)
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);

    if (SDL_GetWindowWMInfo(window, &info) && info.subsystem == SDL_SYSWM_COCOA) {
        NSWindow *window = info.info.cocoa.window;
        CGFloat backingScaleFactor = [window backingScaleFactor];
        return (float)backingScaleFactor;
    }

    return 1.0f;
}
