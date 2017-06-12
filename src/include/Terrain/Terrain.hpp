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
#include "Terrain/Mesher.hpp"
#include "Terrain/VoxelDataGenerator.hpp"
#include "Renderer/GraphicsDevice.hpp"
#include "RenderableStaticMesh.hpp"
#include "TaskDispatcher.hpp"
#include "TerrainMeshes.hpp"
#include <mutex>

// This object represents the terrain of the world.
class Terrain
{
public:
    ~Terrain() = default;
    
    // Constructor.
    Terrain(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
            const std::shared_ptr<TaskDispatcher> &dispatcher);
    
    // No default constructor.
    Terrain() = delete;
    
    // Pass a modelview and projection matrix down for use with the terrain.
    void setTerrainUniforms(const TerrainUniforms &uniforms);
    
    // Draws the terrain.
    void draw(const std::shared_ptr<CommandEncoder> &encoder) const;
    
private:
    std::shared_ptr<VoxelDataStore> _voxels;
    std::shared_ptr<TerrainMeshes> _meshes;
};

#endif /* Terrain_hpp */
