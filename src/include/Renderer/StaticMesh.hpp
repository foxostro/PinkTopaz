//
//  StaticMesh.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/4/16.
//
//

#ifndef StaticMesh_hpp
#define StaticMesh_hpp

#include "Renderer/VertexFormat.hpp"

#include <vector>
#include <glm/mat4x4.hpp>

namespace PinkTopaz::Renderer {
    
    class StaticMesh
    {
    public:
        struct FileVertex
        {
            float position[3];
            unsigned char color[4];
            float texCoord[3];
        };
        
        struct GPUVertex
        {
            glm::vec4 position;
            glm::vec4 texCoord;
            glm::tvec4<uint8_t> color;
        };
        
        struct Header
        {
            uint32_t magic;
            uint32_t version;
            uint32_t w, h, d;
            int32_t numVerts;
            uint32_t len;
            FileVertex vertices[0];
        };
        
        struct Uniforms
        {
            glm::mat4 view, proj;
        };
        
        StaticMesh(const char *filePath);
        ~StaticMesh() = default;
        
        const Header *getHeader() const;
        VertexFormat getVertexFormat() const;
        std::vector<GPUVertex> getVertices() const;
        std::vector<uint8_t> getBufferData() const;
        
        inline size_t getVertexCount() const
        {
            size_t count = getHeader()->numVerts;
            return count;
        }
        
    private:
        std::vector<uint8_t> _bytes;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* StaticMesh_hpp */
