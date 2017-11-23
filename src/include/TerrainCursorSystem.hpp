//
//  TerrainCursorSystem.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 11/20/17.
//
//

#ifndef TerrainCursorSystem_hpp
#define TerrainCursorSystem_hpp

#include "Terrain/Terrain.hpp"
#include "TaskDispatcher.hpp"
#include "TerrainCursor.hpp"
#include "ActiveCamera.hpp"
#include "KeypressEvent.hpp"
#include "MouseMoveEvent.hpp"

#include <entityx/entityx.h>
#include <glm/mat4x4.hpp>

class TerrainCursorSystem : public entityx::System<TerrainCursorSystem>, public entityx::Receiver<TerrainCursorSystem>
{
public:
    TerrainCursorSystem(const std::shared_ptr<TaskDispatcher> &dispatcher);
    void configure(entityx::EventManager &em) override;
    void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override;
    void receive(const entityx::ComponentAddedEvent<ActiveCamera> &event);
    void receive(const entityx::ComponentRemovedEvent<ActiveCamera> &event);
    void receive(const KeypressEvent &event);
    void receive(const MouseMoveEvent &event);
    
private:
    static constexpr size_t maxPlaceDistance = 16;
    
    // Request asynchronous update of the terrain cursor.
    // cursor -- The cursor to update.
    // transform -- The combined camera-terrain transformation.
    // terrain -- The terrain on which the cursor operates.
    void requestCursorUpdate(TerrainCursor &cursor,
                             const glm::mat4 &transform,
                             const std::shared_ptr<Terrain> &terrain);
    
    // Retrieve asynchronously calculated cursor, if results are ready.
    // cursor -- The cursor to update.
    void pollPendingCursorUpdate(TerrainCursor &cursor);
    
    // Calculates the update terrain cursor.
    // Returns the new terrain cursor value, or `none' if request was cancelled.
    // Runs in an aynchronous task on a background thread.
    // cancelled -- Set to true when the request is cancelled.
    // transform -- The combined camera-terrain transformation.
    // terrain -- The terrain on which the cursor operates.
    boost::optional<TerrainCursorValue>
    calcCursor(std::shared_ptr<std::atomic<bool>> cancelled,
               const glm::mat4 &transform,
               const std::shared_ptr<Terrain> &terrain);
    
    std::shared_ptr<TaskDispatcher> _dispatcher;
    entityx::Entity _activeCamera;
    bool _needsUpdate;
};

#endif /* TerrainCursorSystem_hpp */
