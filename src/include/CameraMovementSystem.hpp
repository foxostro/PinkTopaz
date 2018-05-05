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
#include "MouseMoveEvent.hpp"

#include <entityx/entityx.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <unordered_map>
#include "SDL.h"

// System for moving the camera according to keyboard and mouse input.
class CameraMovementSystem : public entityx::System<CameraMovementSystem>, public entityx::Receiver<CameraMovementSystem>
{
public:
    CameraMovementSystem();
    void configure(entityx::EventManager &em) override;
    void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override;
    void receive(const entityx::ComponentAddedEvent<ActiveCamera> &event);
    void receive(const entityx::ComponentRemovedEvent<ActiveCamera> &event);
    void receive(const KeypressEvent &event);
    void receive(const MouseMoveEvent &event);
        
private:
    const float _cameraSpeed, _cameraRotateSpeed;
    const float _mouseSensitivity;
    
    glm::vec3 _eye, _center, _up;
    glm::quat _rotation;
    std::unordered_map<SDL_Keycode, bool> _keys;
    entityx::Entity _activeCamera;
    glm::ivec2 _mousePosDelta;
};

#endif /* CameraMovementSystem_hpp */
