//
//  TerrainProgressSystem.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/5/18.
//
//

#ifndef TerrainProgressSystem_hpp
#define TerrainProgressSystem_hpp

#include <entityx/entityx.h>
#include <unordered_map>
#include <deque>

#include "TerrainProgressEvent.hpp"
#include "RenderableStaticMesh.hpp"
#include "Renderer/GraphicsDevice.hpp"

// System for displaying the progress of terrain generation / loading.
// Terrain chunks in flight will be outlined with a wireframe box.
class TerrainProgressSystem : public entityx::System<TerrainProgressSystem>, public entityx::Receiver<TerrainProgressSystem>
{
public:
    TerrainProgressSystem(const std::shared_ptr<GraphicsDevice> &device);
    void configure(entityx::EventManager &em) override;
    void update(entityx::EntityManager &es,
                entityx::EventManager &events,
                entityx::TimeDelta dt) override;
    void receive(const TerrainProgressEvent &event);
    
private:
    std::shared_ptr<GraphicsDevice> _graphicsDevice;
    std::unordered_map<Morton3, entityx::Entity> _mapCellToEntity;
    std::unordered_map<TerrainProgressEvent::State, glm::vec4> _mapStateToColor;
    std::deque<TerrainProgressEvent> _pendingEvents;
    
    void handleEvent(entityx::EntityManager &es,
                     const TerrainProgressEvent &event);
};

#endif /* TerrainProgressSystem_hpp */
