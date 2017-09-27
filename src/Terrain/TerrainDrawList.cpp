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
   _back(std::make_unique<ArrayType<MaybeMesh>>(box, res), 1)
{}

std::vector<AABB>
TerrainDrawList::draw(const std::shared_ptr<CommandEncoder> &encoder,
                      const Frustum &frustum,
                      const AABB &activeRegion)
{
    updateFrontList(activeRegion);
    
    std::vector<AABB> missingMeshes;
    
    // Draw each cell that is in the camera view-frustum.
    // If the draw list is missing any mesh in the active region then report
    // that to the caller.
    _front.forEachCell(activeRegion, [&](const AABB &cell,
                                         Morton3 index,
                                         const MaybeMesh &maybeMesh){
        if (maybeMesh) {
            const RenderableStaticMesh &drawThis = *maybeMesh;
            if ((drawThis.vertexCount > 0) && frustum.boxIsInside(cell)) {
                encoder->setVertexBuffer(drawThis.buffer, 0);
                encoder->drawPrimitives(Triangles, 0, drawThis.vertexCount, 1);
            }
        } else {
            missingMeshes.push_back(cell);
        }
    });
    
    return missingMeshes;
}

void TerrainDrawList::updateDrawList(const TerrainMesh &mesh)
{
    std::shared_lock<std::shared_mutex> lock(_lock);
    const AABB &cell = mesh.boundingBox();
    _back.writerTransaction(cell, [&](const AABB &cell,
                                      Morton3 index,
                                      MaybeMesh &value){
        value = boost::make_optional(mesh.getMesh());
    });
}

void TerrainDrawList::updateFrontList(const AABB &activeRegion)
{
    std::unique_lock<std::shared_mutex> lock(_lock, std::defer_lock);
    if (!lock.try_lock()) {
        return;
    }
    
    // We can do this operation unlocked because `_lock' ensures there is no
    // concurrent access at this time. Also, this is MUCH faster.
    auto &back = _back.array();
    _front.mutableForEachCell(activeRegion, [&](const AABB &cell,
                                                Morton3 index,
                                                MaybeMesh &dst){
        MaybeMesh &src = back->mutableReference(index);
        dst = src;
    });
}
