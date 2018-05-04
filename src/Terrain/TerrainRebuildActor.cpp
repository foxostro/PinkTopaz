//
//  TerrainRebuildActor.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#include "Terrain/TerrainRebuildActor.hpp"
#include "ThreadName.hpp"
//#include "SDL.h"

// The threshold below which changes in the search point are ignored.
// This prevents re-sorting uneccessarily as all cells fall on a regular grid,
// and small changes will not affect the sorting.
constexpr float searchPointThreshold = 8.f;

TerrainRebuildActor::~TerrainRebuildActor()
{
    _threadShouldExit = true;
    _cvar.notify_all();
    for (auto& thread : _threads) {
        thread.join();
    }
}

TerrainRebuildActor::TerrainRebuildActor(unsigned numThreads,
                                         glm::vec3 initialSearchPoint,
                                         std::function<void(AABB)> processCell)
: _threadShouldExit(false),
  _processCell(processCell),
  _searchPoint(initialSearchPoint)
{
    for (size_t i = 0; i < numThreads; ++i) {
        _threads.emplace_back([this]{
            setNameForCurrentThread("TerrainRebuildActor");
            worker();
        });
    }
}

void TerrainRebuildActor::push(const std::vector<AABB> &cells)
{
    std::lock_guard<std::mutex> lock(_lock);
    
    int numberAdded = 0;
    
    for (const AABB &cell : cells) {
        const auto pair = _set.insert(cell);
        if (pair.second) {
            numberAdded++;
            _cells.push_back(std::make_pair(cell, pair.first));
        }
    }
    
//    SDL_Log("Added %d chunks in push()", numberAdded);
    if (numberAdded > 0) {
        sort();
        _cvar.notify_all();
    }
}

void TerrainRebuildActor::setSearchPoint(glm::vec3 searchPoint)
{
    std::lock_guard<std::mutex> lock(_lock);
    if (glm::distance(_searchPoint, searchPoint) > searchPointThreshold) {
//        SDL_Log("Updating search point from (%.2f, %.2f, %.2f) to (%.2f, %.2f, %.2f)",
//                _searchPoint.x, _searchPoint.y, _searchPoint.z,
//                searchPoint.x, searchPoint.y, searchPoint.z);
        _searchPoint = searchPoint;
        sort();
    }
}

void TerrainRebuildActor::worker()
{
    while (!_threadShouldExit) {
        const auto maybeCell = pop();
        if (maybeCell) {
            _processCell(*maybeCell);
        }
    }
}

boost::optional<AABB> TerrainRebuildActor::pop()
{
    std::unique_lock<std::mutex> lock(_lock);
    _cvar.wait(lock, [this]{
        return _threadShouldExit || !_cells.empty();
    });
    boost::optional<AABB> result;
    if (!(_threadShouldExit || _cells.empty())) {
        const auto [cell, setIter] = _cells.front();
        _cells.pop_front();
        _set.erase(setIter);
        result = boost::make_optional(cell);
    }
    return result;
}

void TerrainRebuildActor::sort()
{
    std::sort(_cells.begin(), _cells.end(),
              [searchPoint=_searchPoint](const auto& pair1, const auto& pair2){
                  const AABB &a = pair1.first;
                  const AABB &b = pair2.first;
                  const auto distA = glm::distance(a.center, searchPoint);
                  const auto distB = glm::distance(b.center, searchPoint);
                  return distA < distB;
              });
}
