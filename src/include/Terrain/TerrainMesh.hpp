//
//  TerrainMesh.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#ifndef TerrainMesh_hpp
#define TerrainMesh_hpp

#include "RenderableStaticMesh.hpp"
#include "Renderer/GraphicsDevice.hpp"
#include "Terrain/VoxelDataStore.hpp"
#include "Terrain/Mesher.hpp"
#include <experimental/optional>

// Terrain is broken up into several meshes. This represents one of the meshes.
class TerrainMesh
{
public:
    using MaybeMesh = typename std::experimental::optional<RenderableStaticMesh>;
    
    ~TerrainMesh() = default;
    
    // Constructor.
    // meshBox -- Bounding box for the associated chunk of terrain.
    // defaultMesh -- Contains resources shared between meshes.
    // graphicsDevice -- Used to create graphics resources for meshes.
    // mesher -- Used to extract an isosurface from the voxel field.
    // voxels -- The voxels fo the world.
    TerrainMesh(const AABB &meshBox,
                const std::shared_ptr<RenderableStaticMesh> &defaultMesh,
                const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                const std::shared_ptr<Mesher> &mesher,
                const std::shared_ptr<VoxelDataStore> &voxels);
    
    // Default constructor is deleted
    TerrainMesh() = delete;
    
    // Copy constructor
    TerrainMesh(const TerrainMesh &mesh);
    
    // Move constructor
    TerrainMesh(TerrainMesh &&mesh);
    
    // Copy-assignment operator
    TerrainMesh& operator=(const TerrainMesh &rhs);
    
    // Returns an optional that contains the mesh, if the mesh is present.
    RenderableStaticMesh getMesh() const;
    
    // Causes the mesh to be rebuilt using the voxel data store.
    void rebuild();
    
    inline const AABB& boundingBox() const
    {
        return _meshBox;
    }

private:
    void rebuildMeshForChunkInner(const Array3D<Voxel> &voxels,
                                  const size_t index,
                                  const AABB &meshBox);
    
    void rebuildMeshForChunkOuter(const size_t index,
                                  const AABB &meshBox);
    
    std::shared_ptr<GraphicsDevice> _graphicsDevice;
    std::shared_ptr<Mesher> _mesher;
    std::shared_ptr<VoxelDataStore> _voxels;
    
    std::shared_ptr<RenderableStaticMesh> _defaultMesh;
    RenderableStaticMesh _mesh;
    AABB _meshBox;
    
    mutable std::mutex _lockMesh;
    std::mutex _lockMeshInFlight;
};

#endif /* TerrainMesh_hpp */
