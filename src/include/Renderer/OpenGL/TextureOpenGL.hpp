//
//  TextureOpenGL.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/26/17.
//
//

#ifndef TextureOpenGL_hpp
#define TextureOpenGL_hpp

#include "Renderer/Texture.hpp"
#include "Renderer/OpenGL/CommandQueue.hpp"
#include "Renderer/OpenGL/opengl.hpp"

namespace PinkTopaz::Renderer::OpenGL {
    
    class TextureOpenGL : public Texture
    {
    public:
        TextureOpenGL(CommandQueue &queue,
                      const TextureDescriptor &desc,
                      const void *data);
        
        virtual ~TextureOpenGL();
        
        inline GLuint getHandle() const { return _handle; }
        
    private:
        GLuint _handle;
        CommandQueue &_commandQueue;
    };
    
} // namespace PinkTopaz::Renderer::OpenGL

#endif /* TextureOpenGL_hpp */
