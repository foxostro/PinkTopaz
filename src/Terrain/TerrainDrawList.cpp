//
//  TerrainDrawList.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#include "Terrain/TerrainDrawList.hpp"

TerrainDrawList::TerrainDrawList(const AABB &box, const glm::ivec3 &res)
 : _front(std::make_unique<ConcurrentGridMutable<MaybeMesh>>(std::make_unique<Array3D<MaybeMesh>>(box, res), 1)),
   _back(std::make_unique<ConcurrentGridMutable<MaybeMesh>>(std::make_unique<Array3D<MaybeMesh>>(box, res), 1))
{}

bool TerrainDrawList::draw(const std::shared_ptr<CommandEncoder> &encoder,
                           const Frustum &frustum,
                           const AABB &activeRegion)
{
    {
        std::unique_lock<std::shared_mutex> lock(_lock, std::defer_lock);
        if (lock.try_lock()) {
            std::swap(_front, _back);
        }
    }
    
    // Draw each cell that is in the camera view-frustum.
    _front->readerTransaction(frustum, [&](const AABB &cell,
                                           Morton3 index,
                                           const MaybeMesh &maybeMesh){
        if (maybeMesh) {
            const RenderableStaticMesh &drawThis = *maybeMesh;
            if (drawThis.vertexCount > 0) {
                encoder->setVertexBuffer(drawThis.buffer, 0);
                encoder->drawPrimitives(Triangles, 0, drawThis.vertexCount, 1);
            }
        }
    });

    // If the draw list is missing any mesh in the active region then report
    // that to the caller. This information will probably be used to adjust the
    // horizon distance so as to control the size of the active region.
    bool anyMissing = false;
    
    _front->readerTransaction(activeRegion, [&](const AABB &cell,
                                                Morton3 index,
                                                const MaybeMesh &maybeMesh){
        if (!maybeMesh) {
            anyMissing = true;
        }
    });
    
    return anyMissing;
}

void TerrainDrawList::updateDrawList(const TerrainMesh &mesh)
{
    std::shared_lock<std::shared_mutex> lock(_lock);
    const AABB &cell = mesh.boundingBox();
    _back->writerTransaction(cell, [&](const AABB &cell,
                                       Morton3 index,
                                       MaybeMesh &value){
        value = std::experimental::make_optional(mesh.getMesh());
    });
}

bool TerrainDrawList::anyMissing(const AABB &activeRegion)
{
    std::shared_lock<std::shared_mutex> lock(_lock);
    bool missing = false;
    _back->readerTransaction(activeRegion, [&](const AABB &cell,
                                               Morton3 index,
                                               const MaybeMesh &value){
        if (!value) {
            missing = true;
        }
    });
    return missing;
}
