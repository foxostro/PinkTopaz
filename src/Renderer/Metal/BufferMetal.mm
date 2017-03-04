//
//  BufferMetal.mm
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#include "Renderer/Metal/BufferMetal.h"
#include "Exception.hpp"

namespace PinkTopaz::Renderer::Metal {

    BufferMetal::BufferMetal(const VertexFormat &format,
                             const std::vector<uint8_t> &bufferData,
                             size_t elementCount,
                             BufferUsage usage,
                             BufferType bufferType)
    : _count(elementCount), _bufferType(bufferType)
    {}
    
    BufferMetal::BufferMetal(const VertexFormat &format,
                             size_t bufferSize,
                             size_t elementCount,
                             BufferUsage usage,
                             BufferType bufferType)
    : _count(elementCount), _bufferType(bufferType)
    {}
    
    BufferMetal::BufferMetal(size_t bufferSize,
                             BufferUsage usage,
                             BufferType bufferType)
    : _count(0), _bufferType(bufferType)
    {}
    
    BufferMetal::BufferMetal(const std::vector<uint8_t> &data,
                             BufferUsage usage,
                             BufferType bufferType)
    : _count(0), _bufferType(bufferType)
    {}
    
    BufferMetal::~BufferMetal() {}
    
    void BufferMetal::replace(const std::vector<uint8_t> &data, size_t count) {}
    
    void BufferMetal::replace(std::vector<uint8_t> &&data, size_t count) {}
    
    void BufferMetal::replace(size_t size, const void *data, size_t count) {}
    
    size_t BufferMetal::getVertexCount() const
    {
        return _count;
    }
    
    BufferType BufferMetal::getType() const
    {
        return _bufferType;
    }

} // namespace PinkTopaz::Renderer::Metal
