//
//  Texture.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/26/17.
//
//

#ifndef Texture_hpp
#define Texture_hpp

#include <cstddef>
#include "Renderer/RendererException.hpp"

enum TextureType
{
    Texture2D,
    Texture2DArray
};

enum TextureFormat
{
    R8,
    RGBA8,
    BGRA8
};

struct TextureDescriptor
{
    TextureType type;
    TextureFormat format;
    size_t width, height, depth;
    int unpackAlignment;
    bool generateMipMaps;
};

// Exception thrown when an unsupported texture type is selected.
class UnsupportedTextureTypeException : public RendererException
{
public:
    UnsupportedTextureTypeException(TextureType type)
    : RendererException("Unsupported texture type {}", (int)type)
    {}
};

// Exception thrown when an unsupported texture format is selected.
class UnsupportedTextureFormatException : public RendererException
{
public:
    UnsupportedTextureFormatException(TextureFormat format)
    : RendererException("Unsupported texture format {}", (int)format)
    {}
};

// Encapsulates a Texture resource in a platform-agnostic manner.
class Texture
{
public:
    virtual ~Texture() = default;
    
protected:
    Texture() = default;
};

#endif /* Texture_hpp */
