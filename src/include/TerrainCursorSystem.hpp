//
//  TerrainCursorSystem.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 11/20/17.
//
//

#ifndef TerrainCursorSystem_hpp
#define TerrainCursorSystem_hpp

#include <entityx/entityx.h>
#include <glm/mat4x4.hpp>
#include "Terrain/Terrain.hpp"
#include "TerrainCursor.hpp"

class TerrainCursorSystem : public entityx::System<TerrainCursorSystem>, public entityx::Receiver<TerrainCursorSystem>
{
public:
    TerrainCursorSystem() = default;
    void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override;
    
private:
    void updateCursor(TerrainCursor &cursor,
                      const glm::mat4 &transform,
                      const std::shared_ptr<Terrain> &terrain);
};

#endif /* TerrainCursorSystem_hpp */
