//
//  FenceMetal.h
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#ifndef FenceMetal_h
#define FenceMetal_h

#ifndef __OBJC__
#error "This is an Objective-C++ header."
#endif

#include "Renderer/Fence.hpp"
#include <Metal/Metal.h>

namespace PinkTopaz::Renderer::Metal {
    
    // Note that MTLFence is currently unavailable on macOS.
    // This is bullshit, but it is what it is, and I have to deal with it.
    class FenceMetal : public Fence
    {
    public:
        FenceMetal();
        virtual ~FenceMetal();
    };
    
} // namespace PinkTopaz::Renderer::Metal

#endif /* FenceMetal_h */
