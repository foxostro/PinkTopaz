//
//  TextureArrayOpenGL.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/5/16.
//
//

#ifndef TextureArrayOpenGL_hpp
#define TextureArrayOpenGL_hpp

#include "Renderer/TextureArray.hpp"
#include "Renderer/OpenGL/CommandQueue.hpp"
#include "Renderer/OpenGL/opengl.hpp"

namespace PinkTopaz::Renderer::OpenGL {
    
    // The TextureArray instance must be used only on the OpenGL thread.
    class TextureArrayOpenGL : public TextureArray
    {
    public:
        TextureArrayOpenGL(const std::shared_ptr<CommandQueue> &commandQueue, const char *fileName);
        
        virtual ~TextureArrayOpenGL();

        inline GLuint getHandle() const { return _handle; }
        
    private:
        GLuint _handle;
        const std::shared_ptr<CommandQueue> &_commandQueue;
    };
    
} // namespace PinkTopaz::Renderer::OpenGL

#endif /* TextureArrayOpenGL_hpp */
