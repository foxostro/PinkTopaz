//
//  TerrainRebuildActor.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#include "Terrain/TerrainRebuildActor.hpp"
#include "ThreadName.hpp"
#include "AutoreleasePool.hpp"
#include "SDL.h" // for SDL_Log()

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
                                         std::shared_ptr<TaskDispatcher> mainThreadDispatcher,
                                         entityx::EventManager &events,
                                         std::function<void(AABB, TerrainProgressTracker&)> processCell)
: _threadShouldExit(false),
  _processCell(processCell),
  _searchPoint(initialSearchPoint),
  _mainThreadDispatcher(mainThreadDispatcher),
  _events(events)
{
    for (size_t i = 0; i < numThreads; ++i) {
        _threads.emplace_back([this]{
            setNameForCurrentThread("TerrainRebuildActor");
            worker();
        });
    }
}

void TerrainRebuildActor::push(const std::vector<std::pair<Morton3, AABB>> &cells, bool insertBack)
{
    std::lock_guard<std::mutex> lock(_lock);
    
    int numberAdded = 0;
    
    for (const auto &cellPair : cells) {
        const Morton3 cellCoords = cellPair.first;
        const AABB &boundingBox = cellPair.second;
        const auto insertResult = _set.insert(boundingBox);
        if (insertResult.second) {
            numberAdded++;
            Cell cell(cellCoords,
                      boundingBox,
                      insertResult.first,
                      _mainThreadDispatcher,
                      _events);
            if (insertBack) {
                _cells.emplace_back(std::move(cell));
            } else {
                _cells.emplace_front(std::move(cell));
            }
        }
    }
    
//    SDL_Log("Added %d chunks in push()", numberAdded);
    if (numberAdded > 0) {
        sort();
        if (numberAdded == 1) {
            _cvar.notify_one();
        } else {
            _cvar.notify_all();
        }
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
        AutoreleasePool pool;
        
        // Wait for a cell to work on.
        boost::optional<Cell> maybeCell;
        {
            std::unique_lock<std::mutex> lock(_lock);
            _cvar.wait(lock, [this]{
                return _threadShouldExit || !_cells.empty();
            });
            if (!(_threadShouldExit || _cells.empty())) {
                Cell cell = std::move(_cells.front());
                _cells.pop_front();
                maybeCell = boost::make_optional(cell);
            }
        }
        
        if (maybeCell) {
            Cell &cell = *maybeCell;
            _processCell(cell.box, cell.progress);
         
            // Remove from the set only at the very end.
            {
                std::unique_lock<std::mutex> lock(_lock);
                cell.progress.setState(TerrainProgressEvent::Complete);
//                cell.progress.dump();
                _set.erase(cell.setIterator);
            }
        }
    }
}

void TerrainRebuildActor::sort()
{
    std::sort(_cells.begin(), _cells.end(),
              [searchPoint=_searchPoint](const Cell& cell1, const Cell& cell2){
                  const AABB &a = cell1.box;
                  const AABB &b = cell2.box;
                  const auto distA = glm::distance(a.center, searchPoint);
                  const auto distB = glm::distance(b.center, searchPoint);
                  return distA < distB;
              });
}
