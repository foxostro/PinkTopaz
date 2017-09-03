//
//  TerrainChunkQueue.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#include "Terrain/TerrainChunkQueue.hpp"

bool TerrainChunkQueue::empty() const
{
    std::lock_guard<std::mutex> lock(_lock);
    return _cells.empty();
}

TerrainChunkQueue::MaybeAABB TerrainChunkQueue::pop()
{
    std::lock_guard<std::mutex> lock(_lock);
    MaybeAABB result;
    if (!_cells.empty()) {
        const AABB cell = _cells.front();
        _cells.pop_front();
        result = make_optional(cell);
    }
    return result;
}

void TerrainChunkQueue::push(const AABB &cell)
{
    std::lock_guard<std::mutex> lock(_lock);
    _cells.push_back(cell);
}

void TerrainChunkQueue::push(const std::vector<AABB> &cells)
{
    std::lock_guard<std::mutex> lock(_lock);
    _cells.insert(_cells.end(), cells.begin(), cells.end());
}
