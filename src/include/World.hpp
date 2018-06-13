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
#include <spdlog/spdlog.h>
#include "Renderer/GraphicsDevice.hpp"
#include "TaskDispatcher.hpp"
#include "Preferences.hpp"

// A World is the same thing as a game zone or level.
// This is a collection of interacting entities and associated systems.
// It is, of course, entirely possible to have multiple worlds. However,
// interactions across worlds are not intended to be routine or easily modeled.
class World : public entityx::EntityX
{
public:
    World() = delete;
    
    World(std::shared_ptr<spdlog::logger> log,
          const Preferences &preferences,
          const std::shared_ptr<GraphicsDevice> &graphicsDevice,
          const std::shared_ptr<TaskDispatcher> &mainThreadDispatcher);
        
    void update(entityx::TimeDelta dt);
    
private:
    std::shared_ptr<spdlog::logger> _log;
    Preferences _preferences;
};

#endif /* World_hpp */
