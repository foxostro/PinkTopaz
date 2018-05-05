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
#include "Morton.hpp"

struct TerrainProgressEvent
{
    enum State {
        Queued,
        WaitingOnVoxels,
        ExtractingSurface,
        Complete
    };
    
    Morton3 cellCoords;
    AABB boundingBox;
    State state;
    
    TerrainProgressEvent() : state(Queued) {}
    
    TerrainProgressEvent(Morton3 c, AABB b, State s)
     : cellCoords(c),
       boundingBox(b),
       state(s)
    {}
    
    TerrainProgressEvent(const TerrainProgressEvent &other) = default;
};

#endif /* TerrainProgressEvent_hpp */
