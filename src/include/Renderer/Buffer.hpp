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

namespace PinkTopaz::Renderer {
    
    enum BufferUsage
    {
        BufferUsageStaticDraw
    };
    
    // Encapsulates a GPU buffer resource in a platform-agnostic manner.
    class Buffer
    {
    public:
        virtual ~Buffer() = default;
        
        // Get the number of vertices contained in the buffer.
        virtual size_t getVertexCount() const = 0;
        
    protected:
        Buffer() = default;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* Buffer_hpp */
