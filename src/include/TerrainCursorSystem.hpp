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

struct TerrainCursor;
struct Transform;
struct TerrainComponent;

class TerrainCursorSystem : public entityx::System<TerrainCursorSystem>, public entityx::Receiver<TerrainCursorSystem>
{
public:
    TerrainCursorSystem() = default;
    void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override;
    
private:
    void updateCursor(TerrainCursor &cursor,
                      const Transform &cameraTransform,
                      const Transform &terrainTransform,
                      const TerrainComponent &terrain);
};

#endif /* TerrainCursorSystem_hpp */
