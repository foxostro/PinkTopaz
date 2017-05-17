//
//  Terrain.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#ifndef Terrain_hpp
#define Terrain_hpp

#include "Terrain/VoxelDataLoader.hpp"
#include "Terrain/VoxelDataStore.hpp"
#include "Terrain/MesherMarchingCubes.hpp"
#include "Renderer/GraphicsDevice.hpp"
#include "RenderableStaticMesh.hpp"
#include <memory>

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
    
    inline const RenderableStaticMesh& getMesh() const { return _mesh; }
    
private:
    RenderableStaticMesh _mesh;
};

#endif /* Terrain_hpp */
