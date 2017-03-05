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

#include <vector>
#include <Metal/Metal.h>

namespace PinkTopaz::Renderer::Metal {
    
    class BufferMetal : public Buffer
    {
    public:
        BufferMetal(id <MTLDevice> device,
                    size_t size,
                    const void *data,
                    BufferUsage usage,
                    BufferType bufferType);
        
        BufferMetal(id <MTLDevice> device,
                    size_t size,
                    BufferUsage usage,
                    BufferType bufferType);
        
        virtual ~BufferMetal();
        
        // Replace the entire contents of the buffer.
        void replace(const std::vector<uint8_t> &data)  override;
        void replace(std::vector<uint8_t> &&data)  override;
        void replace(size_t size, const void *data) override;
        
        BufferType getType() const override;
        
        inline id <MTLBuffer> getMetalBuffer() const { return _buffer; }
        
    private:
        const BufferType _bufferType;
        const BufferUsage _usage;
        id <MTLBuffer> _buffer;
        id <MTLDevice> _device;
    };
    
} // namespace PinkTopaz::Renderer::Metal

#endif /* BufferMetal_h */
