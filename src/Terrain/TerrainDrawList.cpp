//
//  TerrainDrawList.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#include "Terrain/TerrainDrawList.hpp"

TerrainDrawList::TerrainDrawList(const AABB &box, const glm::ivec3 &res)
 : _meshes(std::make_unique<Array3D<RenderableStaticMesh>>(box, res), 1)
{}

void TerrainDrawList::draw(const std::shared_ptr<CommandEncoder> &encoder,
                           const Frustum &frustum)
{
    // Draw each cell that is in the camera view-frustum.
    _meshes.readerTransaction(frustum, [&](const AABB &cell, Morton3 index, const RenderableStaticMesh &drawThis){
        if (drawThis.vertexCount > 0) {
            encoder->setVertexBuffer(drawThis.buffer, 0);
            encoder->drawPrimitives(Triangles, 0, drawThis.vertexCount, 1);
        }
    });
}

void TerrainDrawList::updateDrawList(const TerrainMesh &mesh, const AABB &cell)
{
    _meshes.writerTransaction(cell, [&](const AABB &cell,
                                        Morton3 index,
                                        RenderableStaticMesh &value){
        value = mesh.getMesh();
    });
}
