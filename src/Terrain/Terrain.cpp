//
//  Terrain.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#include "Terrain/Terrain.hpp"

Terrain::Terrain(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
                 const std::shared_ptr<TaskDispatcher> &dispatcher)
 : _voxels(new VoxelDataStore),
   _meshes(new TerrainMeshes(graphicsDevice, dispatcher, _voxels))
{
    // Create an empty writer transaction to trigger the intial building of
    // terrain meshes across the entire grid.
    // AFOX_TODO: We want to remove this and generate meshes on-demand.
    AABB box = _voxels->boundingBox();
    _voxels->writerTransaction(box, [&](GridMutable<Voxel> &voxels){
        return ChangeLog::make("load", box);
    });
}

void Terrain::setTerrainUniforms(const TerrainUniforms &uniforms)
{
    _meshes->setTerrainUniforms(uniforms);
}

void Terrain::draw(const std::shared_ptr<CommandEncoder> &encoder) const
{
    _meshes->draw(encoder);
}
