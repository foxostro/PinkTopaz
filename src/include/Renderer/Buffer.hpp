//
//  Buffer.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/26/17.
//
//

#ifndef Buffer_hpp
#define Buffer_hpp

#include <cstddef>
#include <vector>

namespace PinkTopaz::Renderer {
    
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
        
    protected:
        Buffer() = default;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* Buffer_hpp */
