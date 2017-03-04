//
//  BufferMetal.h
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#ifndef BufferMetal_h
#define BufferMetal_h

#ifndef __OBJC__
#error "This is an Objective-C++ header."
#endif

#include "Renderer/Buffer.hpp"
#include "Renderer/VertexFormat.hpp"

#include <vector>

namespace PinkTopaz::Renderer::Metal {
    
    class BufferMetal : public Buffer
    {
    public:
        BufferMetal(const VertexFormat &format,
                    const std::vector<uint8_t> &bufferData,
                    size_t elementCount,
                    BufferUsage usage,
                    BufferType bufferType);
        
        BufferMetal(const VertexFormat &format,
                    size_t bufferSize,
                    size_t elementCount,
                    BufferUsage usage,
                    BufferType bufferType);
        
        BufferMetal(size_t bufferSize,
                    BufferUsage usage,
                    BufferType bufferType);
        
        BufferMetal(const std::vector<uint8_t> &data,
                    BufferUsage usage,
                    BufferType bufferType);
        
        virtual ~BufferMetal();
        
        // Replace the entire contents of the buffer.
        void replace(const std::vector<uint8_t> &data, size_t count)  override;
        void replace(std::vector<uint8_t> &&data, size_t count)  override;
        void replace(size_t size, const void *data, size_t count) override;
        
        // Get the number of vertices contained in the buffer.
        size_t getVertexCount() const override;
        
        // Gets the type of the buffer.
        BufferType getType() const override;
        
    private:
        size_t _count;
        BufferType _bufferType;
    };
    
} // namespace PinkTopaz::Renderer::Metal

#endif /* BufferMetal_h */
