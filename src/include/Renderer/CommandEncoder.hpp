//
//  CommandEncoder.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/24/17.
//
//

#ifndef CommandEncoder_hpp
#define CommandEncoder_hpp

#include <memory>
#include <functional>
#include <glm/vec4.hpp>

#include "Renderer/Shader.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/TextureSampler.hpp"
#include "Renderer/Buffer.hpp"

enum PrimitiveType
{
    Triangles,
    TriangleStrip
};

enum TriangleFillMode
{
    Fill,
    Lines
};

class CommandEncoder
{
public:
    virtual ~CommandEncoder() {}
    
    // Sets the rendering viewport. This is necessary for computing screen
    // space coordinates. The vector contains the viewport origin in xy and
    // the viewport size in zw.
    virtual void setViewport(const glm::ivec4 &viewport) = 0;
    
    // Binds the specified shader program for use.
    virtual void setShader(const std::shared_ptr<Shader> &shader) = 0;
    
    // Use the texture at the specified index in the fragment program.
    virtual void setFragmentTexture(const std::shared_ptr<Texture> &texture, size_t index) = 0;
    
    // Use the texture sampler at the specified index in the fragment program.
    virtual void setFragmentSampler(const std::shared_ptr<TextureSampler> &sampler, size_t index) = 0;
    
    // Binds the specified buffer for use in the vertex program at the specified index.
    virtual void setVertexBuffer(const std::shared_ptr<Buffer> &buffer, size_t index) = 0;
    
    // Binds the specified buffer for use in the fragment program at the specified index.
    virtual void setFragmentBuffer(const std::shared_ptr<Buffer> &buffer, size_t index) = 0;
    
    // Draw primitives using the bound buffers and other resources.
    virtual void drawPrimitives(PrimitiveType type, size_t first, size_t count, size_t numInstances) = 0;
    
    // Draw indexed primitives using the bound buffers and other resources.
    virtual void
    drawIndexedPrimitives(PrimitiveType primitiveType,
                          size_t indexCount,
                          const std::shared_ptr<Buffer> &indexBuffer,
                          size_t instanceCount) = 0;
    
    // Sets how to rasterize triangle and triangle strip primitives.
    virtual void setTriangleFillMode(TriangleFillMode fillMode) = 0;
    
    // Call when finished encoding to submit to the GPU.
    virtual void commit() = 0;
    
    // Enable/Disable depth testing.
    virtual void setDepthTest(bool enable) = 0;
};

#endif /* CommandEncoder_hpp */
