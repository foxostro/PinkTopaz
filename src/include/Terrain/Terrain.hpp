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
    
    // Gets the most recently generated terrain mesh.
    // For now, the entire terrain has exactly one mesh.
    const RenderableStaticMesh& getMesh() const;
    
private:
    std::shared_ptr<GraphicsDevice> _graphicsDevice;
    std::shared_ptr<Mesher> _mesher;
    std::shared_ptr<VoxelDataStore> _voxels;
    
    mutable std::mutex _lockMesh;
    RenderableStaticMesh _mesh;
    
    void rebuildMesh();
};

#endif /* Terrain_hpp */
