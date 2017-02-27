//
//  BufferOpenGL.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/26/17.
//
//

#ifndef BufferOpenGL_hpp
#define BufferOpenGL_hpp

#include "Renderer/Buffer.hpp"
#include "Renderer/VertexFormat.hpp"
#include "Renderer/OpenGL/CommandQueue.hpp"
#include "Renderer/OpenGL/opengl.hpp"

namespace PinkTopaz::Renderer::OpenGL {
    
    // Encapsulates a GPU buffer resource in a platform-agnostic manner.
    class BufferOpenGL : public Buffer
    {
    public:
        BufferOpenGL(CommandQueue &queue,
                     const VertexFormat &format,
                     const std::vector<uint8_t> &bufferData,
                     size_t count,
                     BufferUsage usage);

        virtual ~BufferOpenGL();
        
        virtual size_t getVertexCount() const override { return _count; }
        
        inline GLuint getHandle() const { return _vao; }
        
    private:
        GLuint _vao;
        size_t _count;
        CommandQueue &_commandQueue;
    };
    
} // namespace PinkTopaz::Renderer::OpenGL

#endif /* BufferOpenGL_hpp */
