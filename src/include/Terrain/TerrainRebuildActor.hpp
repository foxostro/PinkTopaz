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
#include "TerrainProgressTracker.hpp"

// Maintains an ordered list of meshes that need to be generated.
class TerrainRebuildActor
{
public:
    ~TerrainRebuildActor();
    
    TerrainRebuildActor() = delete;
    
    TerrainRebuildActor(unsigned numThreads,
                        glm::vec3 initialSearchPoint,
                        std::shared_ptr<TaskDispatcher> mainThreadDispatcher,
                        entityx::EventManager &events,
                        std::function<void(AABB, TerrainProgressTracker&)> processCell);
    
    // Add a cell to the queue.
    void push(const std::vector<std::pair<Morton3, AABB>> &cells);
    
    // Set the search point.
    void setSearchPoint(glm::vec3 searchPoint);
    
private:
    class Cell
    {
    public:
        AABB box;
        std::unordered_set<AABB>::iterator setIterator;
        TerrainProgressTracker progress;
        
        Cell() = delete;
        
        Cell(Morton3 cellCoords,
             AABB cellBox,
             std::unordered_set<AABB>::iterator iter,
             std::shared_ptr<TaskDispatcher> mainThreadDispatcher,
             entityx::EventManager &events)
         : box(cellBox),
           setIterator(iter),
           progress(cellCoords, cellBox, mainThreadDispatcher, events)
        {}
    };
    
    std::mutex _lock;
    std::condition_variable _cvar;
    std::atomic<bool> _threadShouldExit;
    std::function<void(AABB, TerrainProgressTracker&)> _processCell;
    std::deque<Cell> _cells;
    std::unordered_set<AABB> _set;
    glm::vec3 _searchPoint;
    std::vector<std::thread> _threads;
    std::shared_ptr<TaskDispatcher> _mainThreadDispatcher;
    entityx::EventManager &_events;
    
    // Runs the worker thread.
    void worker();
    
    // Get the cell for the next mesh to generate. Waits for an item to be added
    // to the queue.
    boost::optional<Cell> pop();
    
    // Immediately sort the list. (unlocked)
    void sort();
};

#endif /* TerrainRebuildActor_hpp */
