//
//  TextureMetal.mm
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#include "Renderer/Metal/TextureMetal.h"


static MTLPixelFormat getPixelFormat(TextureFormat format)
{
    switch (format)
    {
        case R8:    return MTLPixelFormatR8Unorm; // The sRGB version is not available for macOS at this time.
        case RGBA8: return MTLPixelFormatBGRA8Unorm_sRGB;
        case BGRA8: return MTLPixelFormatBGRA8Unorm_sRGB;
            
        default:
            throw UnsupportedTextureFormatException(format);
    }
}

static NSUInteger textureDataTypeSize(TextureFormat format)
{
    switch (format)
    {
        case R8: return 1;
        case RGBA8: return 4;
        case BGRA8: return 4;
        default:
            throw UnsupportedTextureFormatException(format);
    }
}

void TextureMetal::initTexture2D(id <MTLDevice> device,
                                 const TextureDescriptor &desc,
                                 const void *data)
{
    MTLPixelFormat pixelFormat = getPixelFormat(desc.format);
    
    MTLTextureDescriptor *metalTextureDescriptor
    = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixelFormat
                                                         width:desc.width
                                                        height:desc.height
                                                     mipmapped:desc.generateMipMaps];
    _texture = [device newTextureWithDescriptor:metalTextureDescriptor];
    MTLRegion region = MTLRegionMake2D(0, 0, desc.width, desc.height);
    NSUInteger pitch = textureDataTypeSize(desc.format) * desc.width;
    [_texture replaceRegion:region
                mipmapLevel:0
                  withBytes:data
                bytesPerRow:pitch];
}

void TextureMetal::initTexture2DArray(id <MTLDevice> device,
                                      const TextureDescriptor &desc,
                                      const void *data)
{
    MTLTextureDescriptor *m = [[MTLTextureDescriptor alloc] init];
    m.textureType       = MTLTextureType2DArray;
    m.pixelFormat       = getPixelFormat(desc.format);
    m.width             = desc.width;
    m.height            = desc.height;
    m.depth             = 1;
    m.mipmapLevelCount  = 1;
    m.sampleCount       = 1;
    m.arrayLength       = desc.depth;
    m.resourceOptions   = MTLResourceStorageModeManaged;
    m.cpuCacheMode      = MTLCPUCacheModeDefaultCache;
    m.storageMode       = MTLStorageModeManaged;
    m.usage             = MTLTextureUsageShaderRead;
    
    _texture = [device newTextureWithDescriptor:m];
    
    MTLRegion region = MTLRegionMake2D(0, 0, desc.width, desc.height);
    NSUInteger c = textureDataTypeSize(desc.format);
    NSUInteger pitch = c * desc.width;
    NSUInteger imageStride = c * desc.width * desc.height;
    
    for (size_t i = 0, n = m.arrayLength; i < n; ++i)
    {
        [_texture replaceRegion:region
                    mipmapLevel:0
                          slice:i
                      withBytes:((const char *)data + i*imageStride)
                    bytesPerRow:pitch
                  bytesPerImage:0];
    }
    
    [m release];
}

TextureMetal::TextureMetal(id <MTLDevice> device,
                           const TextureDescriptor &desc,
                           const void *data)
{
    @autoreleasepool {
        switch (desc.type)
        {
            case Texture2D: initTexture2D(device, desc, data); break;
            case Texture2DArray: initTexture2DArray(device, desc, data); break;
            default:
                throw UnsupportedTextureTypeException(desc.type);
        }
    }
}

TextureMetal::~TextureMetal()
{
    [_texture release];
}
