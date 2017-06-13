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

// Terrain is broken up into several meshes. This represents one of the meshes.
class TerrainMesh
{
public:
    ~TerrainMesh();
    
    TerrainMesh(const AABB &meshBox,
                const std::shared_ptr<RenderableStaticMesh> &defaultMesh,
                const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                const std::shared_ptr<Mesher> &mesher,
                const std::shared_ptr<VoxelDataStore> &voxels);
    
    TerrainMesh(const TerrainMesh &mesh) = default;
    TerrainMesh(TerrainMesh &&mesh) = default;
    TerrainMesh() = delete;
    TerrainMesh& operator=(const TerrainMesh &rhs) = default;
    
    // Passes in uniforms to use when rendering the terrain.
    void setTerrainUniforms(const TerrainUniforms &uniforms);
    
    // Draws this terrain mesh using the specified command encoder.
    // Several resources are set before this call and are reused for all meshes.
    // We expect the caller to take care of that for us.
    void draw(const std::shared_ptr<CommandEncoder> &encoder) const;
    
    // Causes the mesh to be rebuilt using the voxel data store.
    void rebuild();
    
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
};

#endif /* TerrainMesh_hpp */
