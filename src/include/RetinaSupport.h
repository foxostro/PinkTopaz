//
//  osxSupport.h
//  PinkTopaz
//
//  Created by Andrew Fox on 7/1/16.
//
//

#ifndef RetinaSupport_h
#define RetinaSupport_h

#ifdef __cplusplus
extern "C" {
#endif

#include "SDL.h"

float windowScaleFactor(SDL_Window *window);

#ifdef __cplusplus
}
#endif

#endif /* RetinaSupport_h */
