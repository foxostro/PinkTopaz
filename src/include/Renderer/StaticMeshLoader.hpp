//
//  StaticMeshLoader.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/14/17.
//
//

#ifndef StaticMeshLoader_hpp
#define StaticMeshLoader_hpp

#include "Renderer/StaticMesh.hpp"
#include <boost/filesystem.hpp>

namespace PinkTopaz::Renderer {
    
    class StaticMeshLoader
    {
    public:
        struct FileVertex
        {
            float position[3];
            unsigned char color[4];
            float texCoord[3];
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
        
        StaticMeshLoader();
        ~StaticMeshLoader() = default;
        
        StaticMesh load(const boost::filesystem::path &path);
        
    private:
        const uint32_t GEO_MAGIC, GEO_VERSION;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* StaticMesh_hpp */
