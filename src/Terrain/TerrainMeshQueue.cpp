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
    std::lock_guard<std::mutex> lock(_lock);
    return _cells.empty();
}

TerrainMeshQueue::MaybeAABB TerrainMeshQueue::pop(const glm::vec3 &cameraPos)
{
    std::lock_guard<std::mutex> lock(_lock);
    
    float bestDist = std::numeric_limits<float>::infinity();
    auto best = _cells.end();
    
    // Scan to find the cell closest to the camera.
    for(auto iter = _cells.begin(); iter != _cells.end(); ++iter) {
        const AABB &thisCell = *iter;
        const float thisDist = glm::distance(cameraPos, thisCell.center);
        
        if (thisDist < bestDist) {
            best = iter;
            bestDist = thisDist;
        }
    }
    
    // If we found a cell then return it.
    MaybeAABB result;
    if (best != _cells.end()) {
        const AABB cell = *best;
        _cells.erase(best);
        result = std::experimental::make_optional(cell);
    }
    return result;
}

bool TerrainMeshQueue::push(const AABB &cell)
{
    std::lock_guard<std::mutex> lock(_lock);
    auto result = _cells.insert(cell);
    return result.second;
}

bool TerrainMeshQueue::push(const std::vector<AABB> &cells)
{
    bool anyInserted = false;
    std::lock_guard<std::mutex> lock(_lock);
    for (const auto &cell : cells) {
        auto result = _cells.insert(cell);
        anyInserted = anyInserted || result.second;
    }
    return anyInserted;
}
