//
//  TerrainProgressEvent.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/5/18.
//
//

#ifndef TerrainProgressEvent_hpp
#define TerrainProgressEvent_hpp

#include "AABB.hpp"

struct TerrainProgressEvent
{
    enum State {
        Queued,
        WaitingOnVoxels,
        ExtractingSurface,
        Complete
    };
    
    AABB boundingBox;
    State state;
    
    TerrainProgressEvent() : state(Queued) {}
    
    TerrainProgressEvent(AABB b, State s) : boundingBox(b), state(s) {}
    
    TerrainProgressEvent(const TerrainProgressEvent &other) = default;
};

#endif /* TerrainProgressEvent_hpp */
