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

void TerrainDrawList::draw(const std::shared_ptr<CommandEncoder> &encoder)
{
    std::lock_guard<std::mutex> lock(_lockDrawList);
    for (const auto &drawThis : _data) {
        if (drawThis.vertexCount > 0) {
            encoder->setVertexBuffer(drawThis.buffer, 0);
            encoder->drawPrimitives(Triangles, 0, drawThis.vertexCount, 1);
        }
    }
}

void TerrainDrawList::tryUpdateDrawList(const MaybeTerrainMesh &maybeTerrainMesh, const AABB &cell)
{
    if (maybeTerrainMesh) {
        auto maybeRenderableMesh = maybeTerrainMesh->nonblockingGetMesh();
        if (maybeRenderableMesh) {
            std::lock_guard<std::mutex> lock(_lockDrawList);
            _data.set(cell.center, *maybeRenderableMesh);
        }
    }
}

