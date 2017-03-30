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

// Encapsulates a Texture resource in a platform-agnostic manner.
class Texture
{
public:
    virtual ~Texture() = default;
    
protected:
    Texture() = default;
};

#endif /* Texture_hpp */
