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
{}

void Terrain::setTerrainUniforms(const TerrainUniforms &uniforms)
{
    _meshes->setTerrainUniforms(uniforms);
}

void Terrain::draw(const std::shared_ptr<CommandEncoder> &encoder) const
{
    _meshes->draw(encoder);
}
