//
//  TerrainRebuildActor.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#ifndef TerrainRebuildActor_hpp
#define TerrainRebuildActor_hpp

#include <mutex>
#include <deque>
#include <unordered_set>
#include <boost/optional.hpp>
#include <thread>
#include <spdlog/spdlog.h>

#include "TerrainProgressTracker.hpp"

// Maintains an ordered list of meshes that need to be generated.
class TerrainRebuildActor
{
public:
    // A single cell that has been requested to be processed.
    class Cell
    {
    public:
        AABB box;
        std::unordered_set<AABB>::iterator setIterator;
        mutable TerrainProgressTracker progress;
        
        Cell() = delete;
        
        Cell(std::shared_ptr<spdlog::logger> log,
             Morton3 cellCoords,
             AABB cellBox,
             std::unordered_set<AABB>::iterator iter,
             std::shared_ptr<TaskDispatcher> mainThreadDispatcher,
             entityx::EventManager &events,
             std::chrono::steady_clock::time_point appStartTime)
        : box(cellBox),
        setIterator(iter),
        progress(log, cellCoords, cellBox, mainThreadDispatcher,
        events, appStartTime)
        {}
    };
    
    // A batch mesh request which can be queued and processed by this Actor.
    // Mesh requests are batched together. We have a collection of regions,
    // where each one is associated with a bounding box and an index into the
    // mesh grid. In one operation, we request voxels for the union of the
    // bounding boxes and use the result to extract all the specified meshes.
    class Batch
    {
    public:
        Batch(std::vector<Cell> &&requestedCells)
        : _requestedCells(std::move(requestedCells))
        {
            assert(!_requestedCells.empty());
            _boundingBox = _requestedCells.begin()->box;
            for (const Cell &cell : _requestedCells) {
                _boundingBox = _boundingBox.unionBox(cell.box);
            }
        }
        
        inline const AABB& boundingBox() const
        {
            return _boundingBox;
        }
        
        inline const auto& requestedCells() const
        {
            return _requestedCells;
        }
        
        inline auto& requestedCells()
        {
            return _requestedCells;
        }
        
    private:
        AABB _boundingBox;
        std::vector<Cell> _requestedCells;
    };
    
    ~TerrainRebuildActor();
    
    TerrainRebuildActor() = delete;
    
    TerrainRebuildActor(std::shared_ptr<spdlog::logger> log,
                        unsigned numThreads,
                        glm::vec3 initialSearchPoint,
                        std::shared_ptr<TaskDispatcher> mainThreadDispatcher,
                        entityx::EventManager &events,
                        std::chrono::steady_clock::time_point appStartTime,
                        std::function<void(const Batch &)> &&processBatch);
    
    // Add cells to the queue.
    // These will always be popped off the queue in order of increasing distance
    // from the search point.
    void push(const std::vector<std::pair<Morton3, AABB>> &cells);
    
    // Set the search point.
    void setSearchPoint(glm::vec3 searchPoint);
    
private:
    std::mutex _lock;
    std::condition_variable _cvar;
    std::atomic<bool> _threadShouldExit;
    std::function<void(const Batch &)> _processBatch;
    std::deque<Batch> _pendingBatches;
    std::unordered_set<AABB> _set;
    glm::vec3 _searchPoint;
    std::vector<std::thread> _threads;
    std::shared_ptr<TaskDispatcher> _mainThreadDispatcher;
    entityx::EventManager &_events;
    std::shared_ptr<spdlog::logger> _log;
    std::chrono::steady_clock::time_point _appStartTime;
    
    // Runs the worker thread.
    void worker();
    
    // Get the cell for the next mesh to generate. Waits for an item to be added
    // to the queue.
    boost::optional<Cell> pop();
    
    // Immediately sort the list. (unlocked)
    void sort();
};

#endif /* TerrainRebuildActor_hpp */
