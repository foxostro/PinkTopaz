//
//  TerrainCursor.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 11/20/17.
//
//

#ifndef TerrainCursor_hpp
#define TerrainCursor_hpp

#include <glm/vec3.hpp>
#include <boost/thread/future.hpp>
#include <boost/optional.hpp>
#include <memory>
#include <atomic>

// Represents the cursor which selects a block of terrain.
struct TerrainCursorValue
{
    // An inactive cursor is not drawn and cannot be used to place blocks.
    bool active;
    
    // The position of the cursor in the world.
    glm::vec3 pos;
    
    // The place position is usually not the same position where the cursor is
    // drawn. This is usually an empty block immediately before the position.
    glm::vec3 placePos;
    
    TerrainCursorValue() : active(false) {}
};

// Component which holds a terrain cursor value calculated asynchronously.
struct TerrainCursor
{
    std::shared_ptr<std::atomic<bool>> cancelled;
    std::chrono::steady_clock::time_point startTime;
    boost::future<boost::optional<TerrainCursorValue>> pendingValue;
    TerrainCursorValue value;
};

#endif /* TerrainCursor_hpp */
