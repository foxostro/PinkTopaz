//
//  RenderableStaticMesh.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/8/16.
//
//

#ifndef RenderableStaticMesh_hpp
#define RenderableStaticMesh_hpp

#include "Renderer/VertexFormat.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/TextureSampler.hpp"

#include <memory>
#include <glm/vec4.hpp>

// Gives the entity a static mesh which is rendered to represent the entity in the world.
struct RenderableStaticMesh
{
    size_t vertexCount;
    std::shared_ptr<Buffer> buffer;
    std::shared_ptr<Buffer> uniforms;
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Texture> texture;
    std::shared_ptr<TextureSampler> textureSampler;
};

#endif /* RenderableStaticMesh_hpp */
