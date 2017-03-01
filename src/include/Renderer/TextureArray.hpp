//
//  TextureArray.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/5/16.
//
//

#ifndef TextureArray_hpp
#define TextureArray_hpp

namespace PinkTopaz::Renderer {
    
    // Encapsulates a Texture Array resource in a platform-agnostic manner.
    class TextureArray
    {
    public:
        virtual ~TextureArray() = default;
        
    protected:
        TextureArray() = default;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* TextureArray_hpp */
