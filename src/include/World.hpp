//
//  World.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/8/16.
//
//

#ifndef World_hpp
#define World_hpp

#include <entityx/entityx.h>
#include "Renderer/GraphicsDevice.hpp"
#include "TaskDispatcher.hpp"

// A World is the same thing as a game zone or level.
// This is a collection of interacting entities and associated systems.
// It is, of course, entirely possible to have multiple worlds. However,
// interactions across worlds are not intended to be routine or easily modeled.
class World : public entityx::EntityX
{
public:
    World() = delete;
    
    World(const std::shared_ptr<GraphicsDevice> &graphicsDevice,
          const std::shared_ptr<TaskDispatcher> &dispatcherHighPriority,
          const std::shared_ptr<TaskDispatcher> &dispatcherVoxelData,
          const std::shared_ptr<TaskDispatcher> &mainThreadDispatcher);
        
    void update(entityx::TimeDelta dt);
};

#endif /* World_hpp */
