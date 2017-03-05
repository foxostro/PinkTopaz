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
    
    GLenum getBufferTypeEnum(BufferType type);
    
    // Encapsulates a GPU buffer resource in a platform-agnostic manner.
    class BufferOpenGL : public Buffer
    {
    public:
        BufferOpenGL(CommandQueue &queue,
                     const VertexFormat &format,
                     const std::vector<uint8_t> &bufferData,
                     BufferUsage usage,
                     BufferType bufferType);

        BufferOpenGL(CommandQueue &queue,
                     const VertexFormat &format,
                     size_t bufferSize,
                     BufferUsage usage,
                     BufferType bufferType);
        
        BufferOpenGL(CommandQueue &queue,
                     size_t bufferSize,
                     BufferUsage usage,
                     BufferType bufferType);
        
        BufferOpenGL(CommandQueue &queue,
                     const std::vector<uint8_t> &data,
                     BufferUsage usage,
                     BufferType bufferType);

        virtual ~BufferOpenGL();
        
        // Replace the entire contents of the buffer.
        void replace(const std::vector<uint8_t> &wrappedData) override;
        void replace(std::vector<uint8_t> &&wrappedData) override;
        void replace(size_t size, const void *data) override;
        
        inline GLuint getHandleVAO() const { return _vao; }
        inline GLuint getHandleVBO() const { return _vbo; }
        
        inline GLenum getTargetEnum() const
        {
            return getBufferTypeEnum(_bufferType);
        }
        
        // Gets the type of the buffer.
        BufferType getType() const override { return _bufferType; }
        
    private:
        std::mutex _lock;
        GLuint _vao, _vbo;
        GLenum _usage;
        const BufferType _bufferType;
        CommandQueue &_commandQueue;
        
        void setupVertexAttributes(const VertexFormat &format);
        
        void internalCreate(const VertexFormat *format, // optional
                            size_t bufferSize,
                            void *bufferData);
        
        void internalReplace(const void *p, size_t n);
    };
    
} // namespace PinkTopaz::Renderer::OpenGL

#endif /* BufferOpenGL_hpp */
