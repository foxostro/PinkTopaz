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
        glm::mat4 view, proj;
        glm::vec4 color;
    };
    
    // A component that can be attached to an entity to give it a wireframe
    // cube mesh to represent it in the world.
    struct Renderable
    {
        bool hidden;
        size_t vertexCount;
        std::shared_ptr<Buffer> vertexBuffer;
        // TODO: need an index buffer here to accompany the vertex buffer.
        std::shared_ptr<Buffer> uniforms;
        std::shared_ptr<Shader> shader;
        glm::vec4 color;
    };
    
    WireframeCube(std::shared_ptr<GraphicsDevice> graphicsDevice);
    
    Renderable createMesh();
    
private:
    std::shared_ptr<GraphicsDevice> _graphicsDevice;
    Renderable _prototype;
    
    Renderable createPrototypeMesh();
};

#endif /* WireframeCube_hpp */

