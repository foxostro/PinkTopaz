//
//  TerrainDrawList.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#include "Terrain/TerrainDrawList.hpp"

TerrainDrawList::TerrainDrawList(const AABB &box, const glm::ivec3 &res)
 : _data(box, res)
{}

void TerrainDrawList::draw(const std::shared_ptr<CommandEncoder> &encoder,
                           const glm::mat4x4 &modelViewProjection)
{
    // Get the list of cells that are in the camera view-frustum. Use an octree
    // to drastically reduce the number of boxes we have to cull.
    Frustum frustum(modelViewProjection);
    
    // Draw each cell that is in the camera view-frustum.
    std::lock_guard<std::mutex> lock(_lockDrawList);
    _data.forEachCell(frustum, [&](const AABB &cell, Morton3 index, const RenderableStaticMesh &drawThis){
        if (drawThis.vertexCount > 0) {
            encoder->setVertexBuffer(drawThis.buffer, 0);
            encoder->drawPrimitives(Triangles, 0, drawThis.vertexCount, 1);
        }
    });
}

void TerrainDrawList::tryUpdateDrawList(const MaybeTerrainMesh &maybeTerrainMesh, const AABB &cell)
{
    if (maybeTerrainMesh) {
        auto maybeRenderableMesh = maybeTerrainMesh->nonblockingGetMesh();
        if (maybeRenderableMesh) {
            if (_lockDrawList.try_lock()) {
                _data.mutableReference(cell.center) = *maybeRenderableMesh;
                _lockDrawList.unlock();
            }
        }
    }
}

