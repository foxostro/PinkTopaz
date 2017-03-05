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
    
    static MTLSamplerMinMagFilter getMinMagFilter(TextureSamplerFilter filter)
    {
        switch (filter)
        {
            case Nearest:               return MTLSamplerMinMagFilterNearest;
            case Linear:                return MTLSamplerMinMagFilterLinear;
            case NearestMipMapNearest:  return MTLSamplerMinMagFilterNearest;
            case LinearMipMapNearest:   return MTLSamplerMinMagFilterLinear;
            case LinearMipMapLinear:    return MTLSamplerMinMagFilterLinear;
                
            default:
                throw Exception("Unsupported texture min/mag filter.");
        }
    }
    
    static MTLSamplerMipFilter getMipFilter(TextureSamplerFilter filter)
    {
        switch (filter)
        {
            case Nearest:               return MTLSamplerMipFilterNotMipmapped;
            case Linear:                return MTLSamplerMipFilterNotMipmapped;
            case NearestMipMapNearest:  return MTLSamplerMipFilterNotMipmapped; //MTLSamplerMipFilterNearest;
            case LinearMipMapNearest:   return MTLSamplerMipFilterNotMipmapped; //MTLSamplerMipFilterNearest;
            case LinearMipMapLinear:    return MTLSamplerMipFilterNotMipmapped; //MTLSamplerMipFilterLinear;
                
            default:
                throw Exception("Unsupported texture mip filter.");
        }
    }
    
    static MTLSamplerAddressMode getAddressMode(TextureSamplerAddressMode mode)
    {
        switch (mode)
        {
            case ClampToEdge:   return MTLSamplerAddressModeClampToEdge;
            case Repeat:        return MTLSamplerAddressModeRepeat;
                
            default:
                throw Exception("Unsupported texture address mode.");
        }
    }
    
    TextureSamplerMetal::TextureSamplerMetal(id <MTLDevice> device, const TextureSamplerDescriptor &desc)
    {
        MTLSamplerDescriptor *metalSamplerDescriptor = [[MTLSamplerDescriptor alloc] init];
        metalSamplerDescriptor.minFilter = getMinMagFilter(desc.minFilter);
        metalSamplerDescriptor.magFilter = getMinMagFilter(desc.maxFilter);
        metalSamplerDescriptor.mipFilter = getMipFilter(desc.maxFilter);
        metalSamplerDescriptor.sAddressMode = getAddressMode(desc.addressS);
        metalSamplerDescriptor.tAddressMode = getAddressMode(desc.addressT);
        metalSamplerDescriptor.normalizedCoordinates = YES;
        _sampler = [device newSamplerStateWithDescriptor:metalSamplerDescriptor];
        
        [metalSamplerDescriptor release];
    }
    
    TextureSamplerMetal::~TextureSamplerMetal()
    {
        [_sampler release];
    }
    
} // namespace PinkTopaz::Renderer::Metal
