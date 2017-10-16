//
//  Buffer.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/26/17.
//
//

#ifndef Buffer_hpp
#define Buffer_hpp

#include <cstdint>
#include <vector>
#include <string>
    
enum BufferUsage
{
    StaticDraw,
    DynamicDraw
};

enum BufferType
{
    ArrayBuffer,
    UniformBuffer
};

// Encapsulates a GPU buffer resource in a platform-agnostic manner.
class Buffer
{
public:
    virtual ~Buffer() = default;
    
    // Replace the entire contents of the buffer.
    virtual void replace(const std::vector<uint8_t> &data) = 0;
    virtual void replace(std::vector<uint8_t> &&data) = 0;
    virtual void replace(size_t size, const void *data) = 0;
    
    // Gets the type of the buffer.
    virtual BufferType getType() const = 0;
    
    // Adds a debug marker to the buffer for inspecting it in a GPU trace.
    virtual void addDebugMarker(const std::string &marker,
                                size_t location,
                                size_t length) = 0;
    
    // Removes all debug markers from the buffer.
    virtual void removeAllDebugMarkers() = 0;
    
protected:
    Buffer() = default;
};

#endif /* Buffer_hpp */
