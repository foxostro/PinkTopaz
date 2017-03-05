//
//  TextureSamplerMetal.m
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#include "Renderer/Metal/TextureSamplerMetal.h"
#include "Exception.hpp"

namespace PinkTopaz::Renderer::Metal {
    
    TextureSamplerMetal::TextureSamplerMetal(const TextureSamplerDescriptor &desc)
    {
        _sampler = nil;
    }
    
    TextureSamplerMetal::~TextureSamplerMetal()
    {
        [_sampler release];
    }
    
} // namespace PinkTopaz::Renderer::Metal
