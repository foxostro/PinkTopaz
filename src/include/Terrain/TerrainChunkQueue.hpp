//
//  TerrainChunkQueue.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/12/17.
//
//

#ifndef TerrainChunkQueue_hpp
#define TerrainChunkQueue_hpp

#include "AABB.hpp"
#include <mutex>
#include <deque>
#include "optional.hpp"

// Maintains an ordered list of meshes that need to be generated.
class TerrainChunkQueue
{
public:
    using MaybeAABB = typename optional<AABB>;
    
    TerrainChunkQueue() = default;
    
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

#endif /* TerrainChunkQueue_hpp */
