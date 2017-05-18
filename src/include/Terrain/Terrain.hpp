//
//  Terrain.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#ifndef Terrain_hpp
#define Terrain_hpp

#include "Terrain/VoxelDataStore.hpp"
#include "Terrain/MesherMarchingCubes.hpp"
#include "Renderer/GraphicsDevice.hpp"
#include "RenderableStaticMesh.hpp"
#include <mutex>

// This object represents the terrain of the world.
class Terrain
{
public:
    // Constructor.
    Terrain(const std::shared_ptr<GraphicsDevice> &graphicsDevice);
    
    // No default constructor.
    Terrain() = delete;
    
    // Copy constructor is just the default.
    Terrain(const Terrain &terrain) = default;
    
    // Move constructor is just the default.
    Terrain(Terrain &&terrain) = default;
    
    // Destructor is just the default.
    ~Terrain() = default;
    
    // Pass a modelview and projection matrix down for use with the terrain.
    void setTerrainUniforms(const TerrainUniforms &uniforms);
    
    // Draws the terrain.
    void draw(const std::shared_ptr<CommandEncoder> &encoder) const;
    
private:
    static constexpr int MESH_CHUNK_SIZE = 16;
    
    std::shared_ptr<GraphicsDevice> _graphicsDevice;
    std::unique_ptr<Mesher> _mesher;
    std::unique_ptr<VoxelDataStore> _voxels;
    
    mutable std::mutex _lockMeshes;
    std::unique_ptr<Array3D<RenderableStaticMesh>> _meshes;
    RenderableStaticMesh _defaultMesh;
    
    void rebuildMesh(const ChangeLog &changeLog);
};

#endif /* Terrain_hpp */
