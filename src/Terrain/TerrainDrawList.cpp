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
{
    const size_t countLimit = std::pow(512 / 16, 3);
    _back.setCountLimit(countLimit); // TODO: need a better value here
}

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
    
    std::vector<AABB> missingMeshes;
    
    // Draw each cell that is in the camera view-frustum.
    // If the draw list is missing any mesh in the active region then report
    // that to the caller.
    _front.forEachCell(activeRegion, [&](const AABB &cell, Morton3 index){
        boost::optional<MeshPtr> maybeMesh = _front.getIfExists(index);
        bool missing = true;
        
        if (maybeMesh) {
            const MeshPtr &meshPtr = *maybeMesh;
            if (meshPtr) {
                missing = false;
                const RenderableStaticMesh &drawThis = *meshPtr;
                if ((drawThis.vertexCount > 0) && frustum.boxIsInside(cell)) {
                    encoder->setVertexBuffer(drawThis.buffer, 0);
                    encoder->drawPrimitives(Triangles, 0, drawThis.vertexCount, 1);
                }
            }
        }
        
        if (missing) {
            missingMeshes.push_back(cell);
        }
    });
    
    return missingMeshes;
}

void TerrainDrawList::updateDrawList(const TerrainMesh &mesh)
{
    std::shared_lock<std::shared_mutex> lock(_lock);
    const glm::vec3 pos = mesh.boundingBox().center;
    MeshPtr renderable = std::make_shared<RenderableStaticMesh>(mesh.getMesh());
    _back.set(pos, renderable);
}
