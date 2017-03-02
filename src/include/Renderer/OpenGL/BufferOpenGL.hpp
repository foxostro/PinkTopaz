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
                     size_t elementCount,
                     BufferUsage usage);

        BufferOpenGL(CommandQueue &queue,
                     const VertexFormat &format,
                     size_t bufferSize,
                     size_t elementCount,
                     BufferUsage usage);

        virtual ~BufferOpenGL();
        
        // Replace the entire contents of the buffer.
        void replace(size_t size, const void *data) override;
        
        virtual size_t getVertexCount() const override { return _count; }
        
        inline GLuint getHandleVAO() const { return _vao; }
        inline GLuint getHandleVBO() const { return _vbo; }
        
    private:
        std::mutex lock;
        GLuint _vao, _vbo;
        size_t _count;
        GLenum _usage;
        CommandQueue &_commandQueue;
        
        void internalCreate(const VertexFormat &format,
                            size_t bufferSize,
                            void *bufferData);
    };
    
} // namespace PinkTopaz::Renderer::OpenGL

#endif /* BufferOpenGL_hpp */
