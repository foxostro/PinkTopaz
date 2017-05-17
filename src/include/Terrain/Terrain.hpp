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
    std::shared_ptr<GraphicsDevice> _graphicsDevice;
    std::shared_ptr<Mesher> _mesher;
    std::shared_ptr<VoxelDataStore> _voxels;
    
    mutable std::mutex _lockMesh;
    RenderableStaticMesh _mesh;
    
    void rebuildMesh();
};

#endif /* Terrain_hpp */
