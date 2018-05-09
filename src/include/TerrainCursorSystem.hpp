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
#include "Transform.hpp"
#include "ActiveCamera.hpp"
#include "CameraMovedEvent.hpp"
#include "TerrainCursorInvalidatedEvent.hpp"

#include <entityx/entityx.h>
#include <glm/mat4x4.hpp>

// System for updating terrain cursors.
// Entities which have a TerrainCursor component are terrain cursors. These
// permit selection of individual voxels by raycasting from the camera to the
// terrain. These entities are represented on screen via a mesh which the system
// will sync with the cursor position.
class TerrainCursorSystem : public entityx::System<TerrainCursorSystem>, public entityx::Receiver<TerrainCursorSystem>
{
public:
    TerrainCursorSystem(const std::shared_ptr<TaskDispatcher> &dispatcher,
                        const std::shared_ptr<TaskDispatcher> &mainThreadDispatcher);
    void configure(entityx::EventManager &em) override;
    void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override;
    void receive(const entityx::ComponentAddedEvent<ActiveCamera> &event);
    void receive(const entityx::ComponentRemovedEvent<ActiveCamera> &event);
    void receive(const CameraMovedEvent &event);
    void receive(const TerrainCursorInvalidatedEvent &event);
    
private:
    static constexpr size_t maxPlaceDistance = 16;
    
    // Request asynchronous update of the terrain cursor.
    // cameraTerrainTransform -- The combined camera-terrain transformation.
    // terrain -- The terrain on which the cursor operates.
    // cursorEntity -- The entity which represents the terrain cursor.
    void requestCursorUpdate(const glm::mat4 &cameraTerrainTransform,
                             const std::shared_ptr<Terrain> &terrain,
                             entityx::Entity cursorEntity);
    
    std::shared_ptr<TaskDispatcher> _dispatcher;
    std::shared_ptr<TaskDispatcher> _mainThreadDispatcher;
    entityx::Entity _activeCamera;
    bool _needsUpdate;
};

#endif /* TerrainCursorSystem_hpp */
