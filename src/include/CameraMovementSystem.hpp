//
//  CameraMovementSystem.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/5/17.
//
//

#ifndef CameraMovementSystem_hpp
#define CameraMovementSystem_hpp

#include "ActiveCamera.hpp"
#include "KeypressEvent.hpp"

#include <entityx/entityx.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <map>
#include "SDL.h"
    
class CameraMovementSystem : public entityx::System<CameraMovementSystem>, public entityx::Receiver<CameraMovementSystem>
{
public:
    CameraMovementSystem();
    void configure(entityx::EventManager &em) override;
    void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override;
    void receive(const entityx::ComponentAddedEvent<ActiveCamera> &event);
    void receive(const entityx::ComponentRemovedEvent<ActiveCamera> &event);
    void receive(const KeypressEvent &event);
        
private:
    const float _cameraSpeed, _cameraRotateSpeed;
    const float _mouseSensitivity;
    
    glm::vec3 _eye, _center, _up;
    glm::quat _rotation;
    std::map<SDL_Keycode, bool> _keys;
    entityx::Entity _activeCamera;
};

#endif /* CameraMovementSystem_hpp */
