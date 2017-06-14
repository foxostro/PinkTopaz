//
//  TerrainDrawList.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#include "Terrain/TerrainDrawList.hpp"
#include "Frustum.hpp"
#include <glm/glm.hpp>

TerrainDrawList::TerrainDrawList(const AABB &box, const glm::ivec3 &res)
 : _data(box, res)
{}

void TerrainDrawList::draw(const std::shared_ptr<CommandEncoder> &encoder,
                           const glm::mat4x4 &modelViewProjection)
{
    Frustum frustum(modelViewProjection);
    
    std::lock_guard<std::mutex> lock(_lockDrawList);
    
    if (!frustum.inside(_data.boundingBox())) {
        return;
    }
    
    // Draw each cell that is in the camera frustum.
    _data.forEachCell(_data.boundingBox(), [&](const AABB &cell){
        if (frustum.inside(cell)) {
            const auto &drawThis = _data.get(cell.center);
            if (drawThis.vertexCount > 0) {
                encoder->setVertexBuffer(drawThis.buffer, 0);
                encoder->drawPrimitives(Triangles, 0, drawThis.vertexCount, 1);
            }
        }
    });
}

void TerrainDrawList::tryUpdateDrawList(const MaybeTerrainMesh &maybeTerrainMesh, const AABB &cell)
{
    if (maybeTerrainMesh) {
        auto maybeRenderableMesh = maybeTerrainMesh->nonblockingGetMesh();
        if (maybeRenderableMesh) {
            if (_lockDrawList.try_lock()) {
                _data.set(cell.center, *maybeRenderableMesh);
                _lockDrawList.unlock();
            }
        }
    }
}

