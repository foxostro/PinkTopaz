//
//  TerrainVertex.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/5/17.
//
//

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

namespace PinkTopaz::Renderer {

struct TerrainVertex
{
    glm::vec4 position;
    glm::vec4 color;
    glm::vec3 texCoord;
};

struct TerrainUniforms
{
    glm::mat4 view, proj;
};

} // namespace PinkTopaz::Renderer
