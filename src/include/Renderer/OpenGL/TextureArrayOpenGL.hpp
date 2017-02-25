//
//  TextureArrayOpenGL.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/5/16.
//
//

#ifndef TextureArrayOpenGL_hpp
#define TextureArrayOpenGL_hpp

#include <atomic>

#include "Renderer/TextureArray.hpp"
#include "Renderer/OpenGL/CommandQueue.hpp"
#include "Renderer/OpenGL/opengl.hpp"

namespace PinkTopaz::Renderer::OpenGL {
    
    // The TextureArray instance must be used only on the OpenGL thread.
    class TextureArrayOpenGL : public TextureArray
    {
    public:
        TextureArrayOpenGL(CommandQueue &commandQueue, const char *fileName); // TODO: Image loading shouldn't be in the concrete OpenGL implementation.
        
        virtual ~TextureArrayOpenGL();

        GLuint getHandle() const
        {
            return _handle;
        }
        
    private:
        std::atomic<GLuint> _handle;
        CommandQueue &_commandQueue;
    };
    
} // namespace PinkTopaz::Renderer::OpenGL

#endif /* TextureArrayOpenGL_hpp */
