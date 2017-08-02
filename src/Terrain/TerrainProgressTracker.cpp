//
//  TerrainProgressTracker.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 8/1/17.
//
//

#include "Terrain/TerrainProgressTracker.hpp"

std::vector<AABB> TerrainProgressTracker::beginCellsNotInflight(const std::vector<AABB> &cells)
{
    std::lock_guard<std::mutex> lock(_mutex);
    const auto currentTime = std::chrono::steady_clock::now();
    std::vector<AABB> result;
    
    for (const AABB &box : cells) {
        Cell &cell = _cells[box];
        
        if (cell.state != Cell::Inflight) {
            cell.state = Cell::Inflight;
            cell.startTime = currentTime;
            result.push_back(box);
        }
    }
    
    return result;
}

std::chrono::duration<double> TerrainProgressTracker::finish(const AABB &box)
{
    std::lock_guard<std::mutex> lock(_mutex);
    Cell &cell = _cells[box];
    cell.state = Cell::Complete;
    auto currentTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> duration = currentTime - cell.startTime;
    return duration;
}

void TerrainProgressTracker::cancel(const AABB &box)
{
    std::lock_guard<std::mutex> lock(_mutex);
    Cell &cell = _cells[box];
    cell.state = Cell::Cancelled;
}
