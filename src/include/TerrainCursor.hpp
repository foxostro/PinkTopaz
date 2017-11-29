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

// Component which holds a terrain cursor value, calculated asynchronously.
struct TerrainCursor
{
    using Tuple = std::tuple<TerrainCursorValue, std::chrono::steady_clock::time_point>;
    using TaskType = std::shared_ptr<Task<Tuple>>;
    
    // The task gets the updated cursor value and the time the update reqest was
    // issued. (The time can be used to compute the total elapsed time from
    // issuing the request to getting the result.)
    TaskType pending;
    
    // The current value of the terrain cursor.
    TerrainCursorValue value;
};

#endif /* TerrainCursor_hpp */
