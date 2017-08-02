//
//  TerrainMeshQueue.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#ifndef TerrainMeshQueue_hpp
#define TerrainMeshQueue_hpp

#include "AABB.hpp"
#include <mutex>
#include <deque>
#include <experimental/optional>

// Maintains an ordered list of meshes that need to be generated.
class TerrainMeshQueue
{
public:
    using MaybeAABB = typename std::experimental::optional<AABB>;
    
    TerrainMeshQueue() = default;
    
    // Returns true if the queue is empty.
    bool empty() const;
    
    // Get the cell for the next mesh to generate.
    MaybeAABB pop();
    
    // Add another cell to the list. Returns true if the insert succeeded.
    void push(const AABB &cell);
    
    // Add cells to the list.
    void push(const std::vector<AABB> &cells);
    
private:
    mutable std::mutex _lock;
    std::deque<AABB> _cells;
};

#endif /* TerrainMeshQueue_hpp */
