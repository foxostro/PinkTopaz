//
//  TerrainProgressTracker.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 8/1/17.
//
//

#ifndef TerrainProgressTracker_hpp
#define TerrainProgressTracker_hpp

#include "TaskDispatcher.hpp"
#include "TerrainProgressEvent.hpp"
#include "AABB.hpp"
#include "Morton.hpp"

#include <entityx/entityx.h>
#include <spdlog/spdlog.h>
#include <chrono>
#include <mutex>
#include <unordered_map>

// Tracks the progress of a terrain chunk that is being loaded/generated.
class TerrainProgressTracker
{
public:
    TerrainProgressTracker() = delete;
    
    // Immediately starts in the Queued state.
    TerrainProgressTracker(std::shared_ptr<spdlog::logger> log,
                           Morton3 cellCoords,
                           AABB boundingBox,
                           std::shared_ptr<TaskDispatcher> mainThreadDispatcher,
                           entityx::EventManager &events,
                           std::chrono::steady_clock::time_point appStartTime);
    
    // Gets the cell bounding box.
    inline const AABB& getBoundingBox() const
    {
        return _boundingBox;
    }
    
    // Set the state for the specified cell.
    void setState(TerrainProgressEvent::State state);
    
    // Prints to the log information about how long the chunk stayed in each
    // state.
    void dump();
    
private:
    Morton3 _cellCoords;
    AABB _boundingBox;
    TerrainProgressEvent::State _state;
    entityx::EventManager *_events;
    std::shared_ptr<TaskDispatcher> _mainThreadDispatcher;
    std::unordered_map<TerrainProgressEvent::State, std::chrono::steady_clock::time_point> _timeEnteringEachState;
    std::shared_ptr<spdlog::logger> _log;
    std::chrono::steady_clock::time_point _appStartTime;
};

#endif /* TerrainProgressTracker_hpp */
