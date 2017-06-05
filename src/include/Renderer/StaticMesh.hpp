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
#include "Renderer/TerrainVertex.hpp"

#include <vector>

class StaticMesh
{
public:
    // Default constructor. Creates an empty mesh.
    StaticMesh();
    
    // Constructor. Creates a mesh from the lost of vertices.
    StaticMesh(const std::vector<TerrainVertex> &vertices);
    
    ~StaticMesh() = default;
    
    template <class... Args>
    inline void addVertex(Args&&... args)
    {
        _vertices.emplace_back(args...);
    }
    
    inline const std::vector<TerrainVertex>& getVertices() const
    {
        return _vertices;
    }
    
    inline size_t getVertexCount() const
    {
        return _vertices.size();
    }
    
    inline const VertexFormat& getVertexFormat() const
    {
        return _vertexFormat;
    }
    
    std::vector<uint8_t> getBufferData() const;
    
    bool operator==(const StaticMesh &other) const;
    
private:
    void initVertexFormat();
    
    VertexFormat _vertexFormat;
    std::vector<TerrainVertex> _vertices;
};

#endif /* StaticMesh_hpp */
