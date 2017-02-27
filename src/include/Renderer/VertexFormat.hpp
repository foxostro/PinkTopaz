//
//  VertexFormat.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/26/17.
//
//

#ifndef VertexFormat_hpp
#define VertexFormat_hpp

#include <vector>

namespace PinkTopaz::Renderer {
    
    enum AttributeType
    {
        AttributeTypeFloat,
        AttributeTypeUnsignedByte
    };
    
    struct AttributeFormat
    {
        size_t size;
        AttributeType type;
        bool normalized;
        size_t stride;
        size_t offset;
    };
    
    struct VertexFormat
    {
        std::vector<AttributeFormat> attributes;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* VertexFormat_hpp */
