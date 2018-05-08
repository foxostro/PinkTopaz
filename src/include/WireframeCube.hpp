//
//  WireframeCube.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/5/18.
//
//

#ifndef WireframeCube_hpp
#define WireframeCube_hpp

#include <glm/glm.hpp>
#include <entityx/entityx.h>
#include <memory>
#include "Renderer/GraphicsDevice.hpp"
#include "RenderableStaticMesh.hpp"

class WireframeCube
{
public:
    struct Vertex
    {
        glm::vec4 position;
        
        Vertex() : position(0, 0, 0, 0) {}
        
        Vertex(const glm::vec4 &p)
         : position(p)
        {}
        
        inline bool operator==(const Vertex &other) const
        {
            const bool equal = (position == other.position);
            return equal;
        }
    };
    
    struct alignas(16) Uniforms
    {
        glm::mat4 projection;
    };
    
    struct alignas(16) UniformsPerInstance
    {
        glm::mat4 modelView;
        glm::vec4 color;
    };
    
    // A component that can be attached to an entity to give it a wireframe
    // cube mesh to represent it in the world.
    struct Renderable
    {
        bool hidden;
        glm::mat4 modelView;
        glm::vec4 color;
    };
    
    WireframeCube(std::shared_ptr<GraphicsDevice> graphicsDevice);
    
    Renderable createMesh();
    
    // Draws all wireframe cubes. Call this from the RenderSystem.
    void draw(entityx::EntityManager &entityManager,
              const std::shared_ptr<CommandEncoder> &encoder,
              const glm::mat4 &projectionMatrix,
              const glm::mat4 &cameraTransform);
    
private:
    std::shared_ptr<GraphicsDevice> _graphicsDevice;
    size_t _indexCount;
    size_t _maxNumberOfUniformsPerInstancePerUniformBuffer;
    std::shared_ptr<Buffer> _indexBuffer;
    std::shared_ptr<Buffer> _vertexBuffer;
    std::shared_ptr<Buffer> _uniformBuffer;
    std::vector<std::shared_ptr<Buffer>> _uniformsPerInstanceBufferPool;
    std::shared_ptr<Shader> _shader;
    
    Renderable createPrototypeMesh();
};

#endif /* WireframeCube_hpp */

