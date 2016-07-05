//
//  TextureArray.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/5/16.
//
//

#ifndef TextureArray_hpp
#define TextureArray_hpp

#include <OpenGL/gl3.h>

namespace PinkTopaz {
    
    // The TextureArray instance must be used only on the OpenGL thread.
    class TextureArray
    {
    public:
        TextureArray(const char *filePath);
        ~TextureArray();
        
        void bind();

        GLuint getHandle() const
        {
            return _handle;
        }
        
    private:
        GLuint _handle;
    };
    
} // namespace PinkTopaz

#endif /* TextureArray_hpp */
