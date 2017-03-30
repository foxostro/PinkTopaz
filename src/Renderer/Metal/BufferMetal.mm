//
//  BufferMetal.mm
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#include "Renderer/Metal/BufferMetal.h"
#include "Exception.hpp"

static MTLResourceOptions getUsageOption(BufferUsage usage)
{
    switch (usage)
    {
        case StaticDraw:    return MTLResourceStorageModeManaged;
        case DynamicDraw:   return MTLResourceStorageModeManaged;
            
        default:
            throw Exception("Unsupported buffer usage %d\n", (int)usage);
    }
}

BufferMetal::BufferMetal(id <MTLDevice> device,
                         size_t bufferSize,
                         const void *bufferData,
                         BufferUsage usage,
                         BufferType bufferType)
: _bufferType(bufferType), _usage(usage)
{
    _device = [device retain];
    _buffer = [device newBufferWithBytes:bufferData
                                  length:bufferSize
                                 options:getUsageOption(_usage)];
}

BufferMetal::BufferMetal(id <MTLDevice> device,
                         size_t size,
                         BufferUsage usage,
                         BufferType bufferType)
: _bufferType(bufferType), _usage(usage)
{
    _device = [device retain];
    _buffer = [device newBufferWithLength:size
                                  options:getUsageOption(_usage)];
}

BufferMetal::~BufferMetal()
{
    [_buffer release];
    [_device release];
}

void BufferMetal::replace(const std::vector<uint8_t> &data)
{
    replace(data.size(), &data[0]);
}

void BufferMetal::replace(std::vector<uint8_t> &&data)
{
    replace(data.size(), &data[0]);
}

void BufferMetal::replace(size_t size, const void *data)
{
    size_t len = _buffer.length;
    if (size == len) {
        memcpy(_buffer.contents, data, len);
        [_buffer didModifyRange:NSMakeRange(0, len)]; // No API for partial invalidation yet.
    } else {
        [_buffer release];
        _buffer = [_device newBufferWithBytes:data
                                       length:size
                                      options:getUsageOption(_usage)];
    }
}

void BufferMetal::addDebugMarker(const std::string &marker,
                                 size_t location,
                                 size_t length)
{
    [_buffer addDebugMarker:[NSString stringWithUTF8String:marker.c_str()]
                      range:NSMakeRange(location, length)];
}

void BufferMetal::removeAllDebugMarkers()
{
    [_buffer removeAllDebugMarkers];
}

BufferType BufferMetal::getType() const
{
    return _bufferType;
}
