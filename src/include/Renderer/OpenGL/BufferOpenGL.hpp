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

#include "GL/glew.h"

namespace Renderer {
    
    GLenum getBufferTypeEnum(BufferType type);
    
    // Encapsulates a GPU buffer resource in a platform-agnostic manner.
    class BufferOpenGL : public Buffer
    {
    public:
        BufferOpenGL(const std::vector<uint8_t> &bufferData,
                     BufferUsage usage,
                     BufferType bufferType);
        
        BufferOpenGL(size_t bufferSize,
                     const void *bufferData,
                     BufferUsage usage,
                     BufferType bufferType);

        BufferOpenGL(size_t bufferSize,
                     BufferUsage usage,
                     BufferType bufferType);

        virtual ~BufferOpenGL();
        
        // Replace the entire contents of the buffer.
        void replace(const std::vector<uint8_t> &wrappedData) override;
        void replace(std::vector<uint8_t> &&wrappedData) override;
        void replace(size_t size, const void *data) override;
        
        void addDebugMarker(const std::string &marker,
                            size_t location,
                            size_t length) override {}
        
        void removeAllDebugMarkers() override {}
        
        inline GLuint getHandleVAO() const { return _vao; }
        inline GLuint getHandleVBO() const { return _vbo; }
        
        inline GLenum getTargetEnum() const
        {
            return getBufferTypeEnum(_bufferType);
        }
        
        // Gets the type of the buffer.
        BufferType getType() const override { return _bufferType; }
        
    private:
        GLuint _vao, _vbo;
        GLenum _usage;
        const BufferType _bufferType;
        
        void internalCreate(size_t bufferSize, const void *bufferData);
        void internalReplace(size_t bufferSize, const void *bufferData);
    };
    
} // namespace Renderer

#endif /* BufferOpenGL_hpp */
