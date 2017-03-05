//
//  TextureSamplerMetal.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#ifndef TextureSamplerMetal_h
#define TextureSamplerMetal_h

#ifndef __OBJC__
#error "This is an Objective-C++ header."
#endif

#include "Renderer/TextureSampler.hpp"

#include <Metal/Metal.h>

namespace PinkTopaz::Renderer::Metal {
    
    class TextureSamplerMetal : public TextureSampler
    {
    public:
        TextureSamplerMetal(id <MTLDevice> device, const TextureSamplerDescriptor &desc);
        virtual ~TextureSamplerMetal();
        
        inline id <MTLSamplerState> getMetalSampler() const { return _sampler; }
        
    private:
        id <MTLSamplerState> _sampler;
    };
    
} // namespace PinkTopaz::Renderer::Metal

#endif /* TextureSamplerMetal_h */
