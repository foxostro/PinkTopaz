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
#include <cstddef>

#include "Renderer/RendererException.hpp"

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

// Exception thrown when an unsupported buffer attribute type is specified.
class UnsupportedBufferAttributeTypeException : public RendererException
{
public:
    UnsupportedBufferAttributeTypeException(AttributeType type)
    : RendererException("Unsupported buffer attribute type {}", (int)type)
    {}
};

#endif /* VertexFormat_hpp */
