//
//  TerrainProgressTracker.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 8/1/17.
//
//

#ifndef TerrainProgressTracker_hpp
#define TerrainProgressTracker_hpp

#include "AABB.hpp"
#include <chrono>
#include <mutex>
#include <map>
#include <vector>

// Tracks the progress of inflight terrain meshes.
class TerrainProgressTracker
{
public:
    struct Cell
    {
        enum { Uninitialized, Inflight, Complete, Cancelled } state;
        std::chrono::steady_clock::time_point startTime;
        
        Cell() : state(Uninitialized), startTime() {}
    };
    
    // For each cell in the list, if the cell is not infloght then mark is as
    // being inflight. Returns the list of cells for which this was done.
    std::vector<AABB> beginCellsNotInflight(const std::vector<AABB> &cells);
    
    // Marks the cell as being complete. Returns the total elapsed time since
    // the cell went in flight. From now on, the cell will be marked as being
    // Complete.
    std::chrono::duration<double> finish(const AABB &cell);
    
    // Marks the cells as being cancelled.
    void cancel(const AABB &cell);
    
private:
    mutable std::mutex _mutex;
    std::map<AABB, Cell> _cells;
};

#endif /* TerrainProgressTracker_hpp */
