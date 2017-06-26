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
#include <set>
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
    // `cameraPosition' is provided so we can make sure we generate meshes close
    // to the camera before generating meshes further away.
    MaybeAABB pop(const glm::vec3 &cameraPosition);
    
    // Add another cell to the list. Returns true if the insert succeeded.
    // Returns false in the case where the cell was already in the queue.
    bool push(const AABB &cell);
    
    // Add cells to the list. Returns the number of cells successfully inserted.
    size_t push(const std::vector<AABB> &cells);
    
private:
    mutable std::mutex _lock;
    std::set<AABB> _cells;
};

#endif /* TerrainMeshQueue_hpp */
