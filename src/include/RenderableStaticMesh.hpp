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
#include "Renderer/StaticMeshVao.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/TextureArray.hpp"

namespace PinkTopaz {
    
    // Gives the entity a static mesh which is rendered to represent the entity in the world.
    struct RenderableStaticMesh
    {
        RenderableStaticMesh() {}
        
        RenderableStaticMesh(const std::shared_ptr<Renderer::StaticMeshVao> &vao,
                             const std::shared_ptr<Renderer::Shader> &shader,
                             const std::shared_ptr<Renderer::TextureArray> &texture)
        {
            this->vao = vao;
            this->shader = shader;
            this->texture = texture;
        }
        
        std::shared_ptr<Renderer::StaticMeshVao> vao;
        std::shared_ptr<Renderer::Shader> shader;
        std::shared_ptr<Renderer::TextureArray> texture;
    };
    
} // namespace PinkTopaz

#endif /* RenderableStaticMesh_hpp */
