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
        Texture2D
    };
    
    enum TextureFormat
    {
        Red,
    };
    
    struct TextureDescriptor
    {
        TextureType type;
        TextureFormat format;
        size_t width, height;
        int unpackAlignment;
        
        TextureDescriptor()
         : type(Texture2D),
           format(Red),
           width(0),
           height(0),
           unpackAlignment(1)
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
