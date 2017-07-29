//
//  TerrainDrawList.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#include "Terrain/TerrainDrawList.hpp"

TerrainDrawList::TerrainDrawList(const AABB &box, const glm::ivec3 &res)
 : _meshes(box, res)
{}

std::vector<AABB>
TerrainDrawList::draw(const std::shared_ptr<CommandEncoder> &encoder,
                      const Frustum &frustum,
                      const AABB &activeRegion)
{
    std::vector<AABB> missingMeshes;
    
    // AFOX_TODO: Put frustum calling back in place.
    
    // Draw each cell that is in the camera view-frustum.
    // If the draw list is missing any mesh in the active region then report
    // that to the caller.
    _meshes.forEachCell(activeRegion, [&](const AABB &cell,
                                          Morton3 index,
                                          const MaybeMesh &maybeMesh){
        if (maybeMesh) {
            const RenderableStaticMesh &drawThis = *maybeMesh;
            if (drawThis.vertexCount > 0) {
                encoder->setVertexBuffer(drawThis.buffer, 0);
                encoder->drawPrimitives(Triangles, 0, drawThis.vertexCount, 1);
            }
        } else {
            missingMeshes.push_back(cell);
        }
    });
    
    return missingMeshes;
}

void TerrainDrawList::updateDrawList(const std::vector<TerrainMeshRef> &meshes)
{
    for (const auto &ref : meshes) {
        const TerrainMesh &mesh = ref;
        const AABB &cell = mesh.boundingBox();
        auto opt = std::experimental::make_optional(mesh.getMesh());
        _meshes.mutableReference(cell.center) = opt;
    }
}
