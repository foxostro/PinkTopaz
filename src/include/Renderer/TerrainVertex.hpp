//
//  TerrainVertex.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/5/17.
//
//

#ifndef TerrainVertex_hpp
#define TerrainVertex_hpp

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

namespace Renderer {
    
    struct TerrainVertex
    {
        glm::vec4 position;
        glm::vec4 color;
        glm::vec3 texCoord;

        TerrainVertex() : position(0, 0, 0, 0), color(0, 0, 0, 0), texCoord(0, 0, 0) {}

		TerrainVertex(const glm::vec4 &p, const glm::vec4 &c, const glm::vec3 &t)
		 : position(p), color(c), texCoord(t)
		{}
    };
    
    struct TerrainUniforms
    {
        glm::mat4 view, proj;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* TerrainVertex_hpp */
