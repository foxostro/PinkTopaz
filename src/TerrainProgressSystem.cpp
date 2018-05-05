//
//  TerrainProgressSystem.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/5/18.
//
//

#include "TerrainProgressSystem.hpp"
#include "Transform.hpp"
#include "Exception.hpp"
#include <glm/gtx/transform.hpp>

TerrainProgressSystem::TerrainProgressSystem(const std::shared_ptr<GraphicsDevice> &graphicsDevice)
 : _wireframeCube(graphicsDevice)
{
    _mapStateToColor[TerrainProgressEvent::Queued] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    _mapStateToColor[TerrainProgressEvent::WaitingOnVoxels] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    _mapStateToColor[TerrainProgressEvent::ExtractingSurface] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    _mapStateToColor[TerrainProgressEvent::Complete] = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
}

void TerrainProgressSystem::configure(entityx::EventManager &eventManager)
{
    eventManager.subscribe<TerrainProgressEvent>(*this);
}

void TerrainProgressSystem::update(entityx::EntityManager &es,
                                   entityx::EventManager &events,
                                   entityx::TimeDelta deltaMilliseconds)
{
    while (!_pendingEvents.empty()) {
        auto event = _pendingEvents.front();
        handleEvent(es, event);
        _pendingEvents.pop_front();
    }
}

void TerrainProgressSystem::receive(const TerrainProgressEvent &event)
{
    // Queue this to take place in update() where we have access to the
    // entity manager.
    _pendingEvents.push_back(event);
}

void TerrainProgressSystem::handleEvent(entityx::EntityManager &es,
                                        const TerrainProgressEvent &event)
{
    auto iter = _mapCellToEntity.find(event.cellCoords);
    
    if (iter == _mapCellToEntity.end()) {
        // A corresponding entity does not exist. Create it now if the event
        // puts the chunk into a state where we want to show progress.
        if (event.state != TerrainProgressEvent::Complete) {
            entityx::Entity entity = es.create();
            
            Transform transform;
            transform.value = glm::scale(glm::translate(glm::mat4(1), event.boundingBox.center), event.boundingBox.extent*2.f);
            entity.assign<Transform>(transform);
            
            // TODO: Render an instanced wireframe box for each chunk. If we do this then each box only consumes one vec4 for the position and one vec4 for the color.
            auto mesh = _wireframeCube.createMesh();
            entity.assign<WireframeCube::Renderable>(mesh);
            
            _mapCellToEntity[event.cellCoords] = std::move(entity);
        }
    } else {
        entityx::Entity entity = iter->second;
        
        entity.component<WireframeCube::Renderable>()->color = _mapStateToColor[event.state];
        
        // If the chunk is "complete" then remove the entity.
        // We no longer need the wireframe box.
        if (event.state == TerrainProgressEvent::Complete) {
            entity.destroy();
            
            auto iter = _mapCellToEntity.find(event.cellCoords);
            
            if (iter != _mapCellToEntity.end()) {
                _mapCellToEntity.erase(iter);
            } else {
                throw Exception("How did we get an invalid iterator here?");
            }
        }
    }
}
