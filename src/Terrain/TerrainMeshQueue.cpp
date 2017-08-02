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

TerrainMeshQueue::MaybeAABB TerrainMeshQueue::pop()
{
    std::lock_guard<std::mutex> lock(_lock);
    MaybeAABB result;
    if (!_cells.empty()) {
        const AABB cell = _cells.front();
        _cells.pop_front();
        result = std::experimental::make_optional(cell);
    }
    return result;
}

void TerrainMeshQueue::push(const AABB &cell)
{
    std::lock_guard<std::mutex> lock(_lock);
    _cells.push_back(cell);
}

void TerrainMeshQueue::push(const std::vector<AABB> &cells)
{
    std::lock_guard<std::mutex> lock(_lock);
    _cells.insert(_cells.end(), cells.begin(), cells.end());
}
