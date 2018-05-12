//
//  TextureSampler.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/2/17.
//
//

#ifndef TextureSampler_hpp
#define TextureSampler_hpp

#include "Renderer/RendererException.hpp"

enum TextureSamplerAddressMode
{
    Repeat,
    ClampToEdge
};

enum TextureSamplerFilter
{
    Nearest,
    Linear,
    NearestMipMapNearest,
    LinearMipMapNearest,
    LinearMipMapLinear
};

struct TextureSamplerDescriptor
{
    TextureSamplerAddressMode addressS, addressT;
    TextureSamplerFilter minFilter, maxFilter;
};

// Exception thrown when an unsupported texture sampler address mode is used.
class UnsupportedTextureSamplerAddressModeException : public RendererException
{
public:
    UnsupportedTextureSamplerAddressModeException(TextureSamplerAddressMode mode)
    : RendererException("Unsupported texture sampler address mode {}", (int)mode)
    {}
};

// Exception thrown when an unsupported texture sampler filter is used.
class UnsupportedTextureSamplerFilterException : public RendererException
{
public:
    UnsupportedTextureSamplerFilterException(TextureSamplerFilter filter)
    : RendererException("Unsupported texture sampler filter {}", (int)filter)
    {}
};

// Encapsulates a texture sampler resource in a platform-agnostic manner.
class TextureSampler
{
public:
    virtual ~TextureSampler() = default;
    
protected:
    TextureSampler() = default;
};

#endif /* TextureSampler_hpp */
