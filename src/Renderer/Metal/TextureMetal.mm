//
//  TextureMetal.mm
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#include "Renderer/Metal/TextureMetal.h"
#include "Exception.hpp"

namespace PinkTopaz::Renderer::Metal {
    
    static MTLPixelFormat getPixelFormat(TextureFormat format)
    {
        switch (format)
        {
            case R8:    return MTLPixelFormatR8Unorm;
            case RGBA8: return MTLPixelFormatRGBA8Unorm;
            case BGRA8: return MTLPixelFormatBGRA8Unorm;
            
            default:
                throw Exception("Unsupported pixel format.");
        }
    }

    TextureMetal::TextureMetal(id <MTLDevice> device,
                               const TextureDescriptor &desc,
                               const void *data)
    {
        @autoreleasepool {
            MTLPixelFormat pixelFormat = getPixelFormat(desc.format);
            MTLTextureDescriptor *metalTextureDescriptor
            = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixelFormat
                                                                 width:desc.width
                                                                height:desc.height
                                                             mipmapped:desc.generateMipMaps];
            _texture = [device newTextureWithDescriptor:metalTextureDescriptor];
        }
    }
    
    TextureMetal::~TextureMetal()
    {
        [_texture release];
    }

} // namespace PinkTopaz::Renderer::Metal
