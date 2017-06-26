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
        auto iter = _cells.begin();
        const AABB cell = *iter;
        _cells.erase(iter);
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

size_t TerrainMeshQueue::push(const std::vector<AABB> &cells)
{
    size_t numberInserted = 0;
    std::lock_guard<std::mutex> lock(_lock);
    for (const auto &cell : cells) {
        auto result = _cells.insert(cell);
        numberInserted += (result.second) ? 1 : 0;
    }
    return numberInserted;
}
