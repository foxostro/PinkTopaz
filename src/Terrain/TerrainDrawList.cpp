//
//  TerrainDrawList.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#include "Terrain/TerrainDrawList.hpp"

TerrainDrawList::TerrainDrawList(const AABB &box, const glm::ivec3 &res)
 : _front(box, res),
   _back(box, res)
{}

std::vector<AABB>
TerrainDrawList::draw(const std::shared_ptr<CommandEncoder> &encoder,
                      const Frustum &frustum,
                      const AABB &activeRegion)
{
    // If we can, copy the back draw list into the front list.
    {
        std::unique_lock<std::shared_mutex> lock(_lock, std::defer_lock);
        if (lock.try_lock()) {
            _front = _back;
        }
    }
    
    // Draw each cell that is in the camera view-frustum.
    for (const auto &cellCoords : _front.slice(frustum, activeRegion)) {
        const Morton3 index = _front.indexAtCellCoords(cellCoords);
        boost::optional<MeshPtr> maybeMesh = _front.getIfExists(index);
        
        if (maybeMesh && *maybeMesh) {
            const RenderableStaticMesh &drawThis = **maybeMesh;
            if (drawThis.vertexCount > 0) {
                encoder->setVertexBuffer(drawThis.buffer, 0);
                encoder->drawPrimitives(Triangles, 0, drawThis.vertexCount, 1);
            }
        }
    }
    
    std::vector<AABB> missingMeshes;
    
    // If the draw list is missing any mesh in the active region then report
    // that to the caller.
    for (const auto cellCoords : _front.slice(activeRegion)) {
        const Morton3 index = _front.indexAtCellCoords(cellCoords);
        boost::optional<MeshPtr> maybeMesh = _front.getIfExists(index);
        const bool notMissing = maybeMesh && *maybeMesh;
        if (!notMissing) {
            const AABB cell = _front.cellAtCellCoords(cellCoords);
            missingMeshes.push_back(cell);
        }
    }
    
    return missingMeshes;
}

void TerrainDrawList::updateDrawList(const TerrainMesh &mesh)
{
    std::shared_lock<std::shared_mutex> lock(_lock);
    const glm::vec3 pos = mesh.boundingBox().center;
    MeshPtr renderable = std::make_shared<RenderableStaticMesh>(mesh.getMesh());
    _back.set(pos, renderable);
}

void TerrainDrawList::setCountLimit(unsigned countLimit)
{
    std::unique_lock<std::shared_mutex> lock(_lock);
    _front.setCountLimit(countLimit);
    _back.setCountLimit(countLimit);
}
