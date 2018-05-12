//
//  TerrainProgressTracker.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 8/1/17.
//
//

#include "Terrain/TerrainProgressTracker.hpp"

TerrainProgressTracker::TerrainProgressTracker(std::shared_ptr<spdlog::logger> log,
                                               Morton3 cellCoords,
                                               AABB boundingBox,
                                               std::shared_ptr<TaskDispatcher> mainThreadDispatcher,
                                               entityx::EventManager &events)
: _cellCoords(cellCoords),
  _boundingBox(boundingBox),
  _state(TerrainProgressEvent::Queued),
  _events(&events),
  _mainThreadDispatcher(mainThreadDispatcher),
  _log(log)
{
    setState(TerrainProgressEvent::Queued);
}

void TerrainProgressTracker::setState(TerrainProgressEvent::State state)
{
    _state = state;
    _timeEnteringEachState[state] = std::chrono::steady_clock::now();
    _mainThreadDispatcher->async([events=_events,
                                  cellCoords=_cellCoords,
                                  box=_boundingBox,
                                  state=_state]{
        events->emit(TerrainProgressEvent(cellCoords, box, state));
    });
}

void TerrainProgressTracker::dump()
{
    assert(_state == TerrainProgressEvent::Complete);
    
    auto timeElapsed = [&](TerrainProgressEvent::State endState, TerrainProgressEvent::State beginState){
        const auto beginTime = _timeEnteringEachState[beginState];
        const auto endTime = _timeEnteringEachState[endState];
        const auto duration = endTime - beginTime;
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        std::string msStr = std::to_string(ms.count());
        return msStr;
    };
    
    std::string timeQueuedStr = timeElapsed(TerrainProgressEvent::WaitingOnVoxels,
                                            TerrainProgressEvent::Queued);
    std::string timeWaitingForVoxelsStr = timeElapsed(TerrainProgressEvent::ExtractingSurface,
                                                      TerrainProgressEvent::WaitingOnVoxels);
    std::string timeExtractingSurfaceStr = timeElapsed(TerrainProgressEvent::Complete,
                                                       TerrainProgressEvent::ExtractingSurface);
    std::string timeTotalElapsedStr = timeElapsed(TerrainProgressEvent::Complete,
                                                  TerrainProgressEvent::Queued);
    
    _log->trace("Cell {}:\n"
                "\tQueued             -- {} ms\n"
                "\tWaitingOnVoxels    -- {} ms\n"
                "\tExtractingSurface  -- {} ms\n"
                "\tTotal Elapsed Time -- {} ms",
                _boundingBox,
                timeQueuedStr,
                timeWaitingForVoxelsStr,
                timeExtractingSurfaceStr,
                timeTotalElapsedStr);
}
