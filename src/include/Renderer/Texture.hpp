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

namespace PinkTopaz::Renderer {
    
    enum TextureType
    {
        Texture2D,
        Texture2DArray
    };
    
    enum TextureFormat
    {
        R8,
        RGBA8
    };
    
    enum TextureWrapMode
    {
        Repeat,
        ClampToEdge
    };
    
    enum TextureFilter
    {
        Nearest,
        Linear
    };
    
    struct TextureDescriptor
    {
        TextureType type;
        TextureFormat format;
        size_t width, height, depth;
        int unpackAlignment;
        TextureWrapMode wrapS, wrapT;
        TextureFilter minFilter, maxFilter;
        
        TextureDescriptor()
         : type(Texture2D),
           format(RGBA8),
           width(0),
           height(0),
           depth(0),
           unpackAlignment(4),
           wrapS(Repeat),
           wrapT(Repeat),
           minFilter(Linear),
           maxFilter(Linear)
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
    
} // namespace PinkTopaz::Renderer

#endif /* Texture_hpp */
