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
#include <glm/gtx/string_cast.hpp>
#include <unordered_map>

#include <boost/functional/hash.hpp>
namespace std {
    template <> struct hash<glm::ivec2>
    {
        size_t operator()(const glm::ivec2 &vec) const
        {
            std::hash<glm::ivec2::value_type> hasher;
            size_t seed = 0;
            boost::hash_combine(seed, hasher(vec.x));
            boost::hash_combine(seed, hasher(vec.y));
            return seed;
        }
    };
}

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

TerrainRebuildActor::TerrainRebuildActor(std::shared_ptr<spdlog::logger> log,
                                         unsigned numThreads,
                                         glm::vec3 initialSearchPoint,
                                         std::shared_ptr<TaskDispatcher> mainThreadDispatcher,
                                         entityx::EventManager &events,
                                         std::chrono::steady_clock::time_point appStartTime,
                                         std::function<void(const Batch &)> &&processBatch)
: _threadShouldExit(false),
  _processBatch(std::move(processBatch)),
  _searchPoint(initialSearchPoint),
  _mainThreadDispatcher(mainThreadDispatcher),
  _events(events),
  _log(log),
  _appStartTime(appStartTime)
{
    for (size_t i = 0; i < numThreads; ++i) {
        _threads.emplace_back([this]{
            setNameForCurrentThread("TerrainRebuildActor");
            worker();
        });
    }
}

void TerrainRebuildActor::push(const std::vector<std::pair<Morton3, AABB>> &cells)
{
    std::lock_guard<std::mutex> lock(_lock);
    
    int numberAdded = 0;
    std::unordered_map<glm::ivec2, std::vector<Cell>> mapColumnToCells;
    
    for (const auto &cellPair : cells) {
        const Morton3 cellIndex = cellPair.first;
        const AABB &boundingBox = cellPair.second;
        const auto insertResult = _set.insert(boundingBox);
        if (insertResult.second) {
            numberAdded++;
            
            const glm::ivec3 cellCoords = cellIndex.decode();
            const glm::ivec2 columnCoords(cellCoords.x, cellCoords.z);
            
            auto iter = mapColumnToCells.find(columnCoords);
            if (iter == mapColumnToCells.end()) {
                mapColumnToCells.insert(std::make_pair(columnCoords, std::vector<Cell>{}));
                iter = mapColumnToCells.find(columnCoords);
            }
            
            iter->second.emplace_back(_log,
                                      cellIndex,
                                      boundingBox,
                                      insertResult.first,
                                      _mainThreadDispatcher,
                                      _events,
                                      _appStartTime);
        }
    }
    
    _log->trace("Added {} chunks in push()", numberAdded);
    if (numberAdded > 0) {
        for (auto &pair : mapColumnToCells) {
            _pendingBatches.emplace_back(std::move(pair.second));
        }
        sort();
        if (mapColumnToCells.size() == 1) {
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
        _log->trace("Updating search point from {} to {}",
                    glm::to_string(_searchPoint), glm::to_string(searchPoint));
        _searchPoint = searchPoint;
        sort();
    }
}

void TerrainRebuildActor::worker()
{
    while (!_threadShouldExit) {
        AutoreleasePool pool;
        
        // Wait for a cell to work on.
        boost::optional<Batch> maybeBatch;
        {
            std::unique_lock<std::mutex> lock(_lock);
            _cvar.wait(lock, [this]{
                return _threadShouldExit || !_pendingBatches.empty();
            });
            if (!(_threadShouldExit || _pendingBatches.empty())) {
                maybeBatch = boost::make_optional(std::move(_pendingBatches.front()));
                _pendingBatches.pop_front();
            }
        }
        
        if (maybeBatch) {
            Batch &batch = *maybeBatch;
            _processBatch(batch);
         
            // Remove from the set only at the very end.
            std::unique_lock<std::mutex> lock(_lock);
            for (Cell &cell : batch.requestedCells())
            {
                cell.progress.setState(TerrainProgressEvent::Complete);
                cell.progress.dump();
                _set.erase(cell.setIterator);
            }
        }
    }
}

void TerrainRebuildActor::sort()
{
    std::sort(_pendingBatches.begin(), _pendingBatches.end(),
              [searchPoint=_searchPoint](const Batch& batch1, const Batch& batch2){
                  const AABB &a = batch1.boundingBox();
                  const AABB &b = batch2.boundingBox();
                  const auto distA = glm::distance(a.center, searchPoint);
                  const auto distB = glm::distance(b.center, searchPoint);
                  return distA < distB;
              });
}
