//
//  TerrainCursor.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 11/20/17.
//
//

#ifndef TerrainCursor_hpp
#define TerrainCursor_hpp

#include <functional>
#include <glm/vec3.hpp>
#include <boost/optional.hpp>
#include "TaskDispatcher.hpp"

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

// Component which holds a terrain cursor value, updated asynchronously.
struct TerrainCursor
{
    // The cursor value is read and updated atomically on multiple threads.
    std::atomic<TerrainCursorValue> value;
    
    // If there is a pending task to update the cursor asynchronously then
    // calling this function will cancel that task.
    std::function<void()> canceller;
    
    TerrainCursor() : canceller([]{}) {}
};

#endif /* TerrainCursor_hpp */
