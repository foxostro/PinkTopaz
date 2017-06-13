//
//  TerrainMeshQueue.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#include "Terrain/TerrainMeshQueue.hpp"

bool TerrainMeshQueue::empty() const
{
    return _cells.empty();
}

TerrainMeshQueue::MaybeAABB TerrainMeshQueue::pop(const glm::vec3 &cameraPos)
{
    std::lock_guard<std::mutex> lock(_lock);
    
    auto best = _cells.begin();
    
    // Scan to find the cell closest to the camera.
    for(auto iter = _cells.begin(); iter != _cells.end(); ++iter) {
        const AABB &thisCell = *iter;
        const AABB &bestCell = *best;
        const float thisDist = glm::distance(cameraPos, thisCell.center);
        const float bestDist = glm::distance(cameraPos, bestCell.center);
        
        if (thisDist < bestDist) {
            best = iter;
        }
    }
    
    // If we found a cell then return it.
    MaybeAABB result;
    if (best != _cells.end()) {
        _cells.erase(best);
        result = std::experimental::make_optional(*best);
    }
    return result;
}

bool TerrainMeshQueue::push(const AABB &cell)
{
    std::lock_guard<std::mutex> lock(_lock);
    auto result = _cells.insert(cell);
    return result.second;
}
