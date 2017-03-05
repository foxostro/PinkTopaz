//
//  RenderableStaticMesh.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/8/16.
//
//

#ifndef RenderableStaticMesh_hpp
#define RenderableStaticMesh_hpp

#include <memory>
#include "Renderer/Buffer.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/TextureSampler.hpp"

namespace PinkTopaz {
    
    // Gives the entity a static mesh which is rendered to represent the entity in the world.
    struct RenderableStaticMesh
    {
        size_t vertexCount;
        std::shared_ptr<Renderer::Buffer> buffer;
        std::shared_ptr<Renderer::Buffer> uniforms;
        std::shared_ptr<Renderer::Shader> shader;
        std::shared_ptr<Renderer::Texture> texture;
        std::shared_ptr<Renderer::TextureSampler> textureSampler;
    };
    
} // namespace PinkTopaz

#endif /* RenderableStaticMesh_hpp */
