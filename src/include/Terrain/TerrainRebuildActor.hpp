//
//  TerrainRebuildActor.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#ifndef TerrainRebuildActor_hpp
#define TerrainRebuildActor_hpp

#include "AABB.hpp"
#include <mutex>
#include <deque>
#include <unordered_set>
#include <boost/optional.hpp>
#include <thread>

// Maintains an ordered list of meshes that need to be generated.
class TerrainRebuildActor
{
public:
    ~TerrainRebuildActor();
    
    TerrainRebuildActor() = delete;
    
    TerrainRebuildActor(unsigned numThreads,
                        glm::vec3 initialSearchPoint,
                        std::function<void(AABB)> processCell);
    
    // Add cells to the queue.
    void push(const std::vector<AABB> &cells);
    
    // Set the search point.
    void setSearchPoint(glm::vec3 searchPoint);
    
private:
    std::mutex _lock;
    std::condition_variable _cvar;
    std::atomic<bool> _threadShouldExit;
    std::function<void(AABB)> _processCell;
    std::deque<std::pair<AABB, std::unordered_set<AABB>::iterator>> _cells;
    std::unordered_set<AABB> _set;
    glm::vec3 _searchPoint;
    std::vector<std::thread> _threads;
    
    // Runs the worker thread.
    void worker();
    
    // Get the cell for the next mesh to generate. Waits for an item to be added
    // to the queue.
    boost::optional<AABB> pop();
    
    // Immediately sort the list. (unlocked)
    void sort();
};

#endif /* TerrainRebuildActor_hpp */
