//
//  CameraMovementSystem.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/5/17.
//
//

#include "CameraMovementSystem.hpp"
#include "Transform.hpp"
#include "Exception.hpp"

#include <glm/gtc/matrix_transform.hpp>
    
CameraMovementSystem::CameraMovementSystem()
 : _cameraSpeed(50.0f),
   _cameraRotateSpeed(1.0f),
   _mouseSensitivity(5.0f),
   _mouseEventPending(false)
{}
    
void CameraMovementSystem::configure(entityx::EventManager &em)
{
    em.subscribe<entityx::ComponentAddedEvent<ActiveCamera>>(*this);
    em.subscribe<entityx::ComponentRemovedEvent<ActiveCamera>>(*this);
    em.subscribe<KeypressEvent>(*this);
    em.subscribe<MouseMoveEvent>(*this);
}
    
void CameraMovementSystem::update(entityx::EntityManager &es,
                                    entityx::EventManager &events,
                                    entityx::TimeDelta deltaMilliseconds)
{
    const entityx::TimeDelta dt = deltaMilliseconds / 1000;
        
    if (!_activeCamera.valid()) {
        return;
    }
        
    const glm::vec3 localForward(0, 0, -1);
    const glm::vec3 localRight(1, 0, 0);
    const glm::vec3 localUp(0, 1, 0);
        
    const glm::vec3 worldForward = glm::normalize(_rotation * localForward);
    const glm::vec3 worldRight = glm::normalize(_rotation * localRight);
    const glm::vec3 worldUp = glm::normalize(_rotation * localUp);
        
    glm::vec3 velocity;
        
    const float angle = _cameraRotateSpeed*dt;
    const float speed = _cameraSpeed*dt;
        
    if(_keys[SDLK_w]) {
        velocity += worldForward * speed;
    } else if(_keys[SDLK_s]) {
        velocity += worldForward * -speed;
    }
        
    if(_keys[SDLK_a]) {
        velocity += worldRight * -speed;
    } else if(_keys[SDLK_d]) {
        velocity += worldRight * speed;
    }
        
    if(_keys[SDLK_z]) {
        velocity += localUp * -speed;
    } else if(_keys[SDLK_x]) {
        velocity += localUp * speed;
    }

    if(_keys[SDLK_i]) {
        _rotation = glm::angleAxis(-angle, worldRight) * _rotation;
    } else if(_keys[SDLK_k]) {
        _rotation = glm::angleAxis(angle, worldRight) * _rotation;
    }
        
    if(_keys[SDLK_j]) {
        glm::quat deltaRot = glm::angleAxis(angle, localUp);
        _rotation = deltaRot * _rotation;
    } else if(_keys[SDLK_l]) {
        glm::quat deltaRot = glm::angleAxis(-angle, localUp);
        _rotation = deltaRot * _rotation;
    }
    
    int mouseDeltaX = 0, mouseDeltaY = 0;
    
    if (_mouseEventPending) {
        _mouseEventPending = false;
        SDL_GetRelativeMouseState(&mouseDeltaX, &mouseDeltaY);
    }
        
    if(mouseDeltaX != 0) {
        float mouseDirectionX = -mouseDeltaX*_mouseSensitivity*dt;
        float angle = mouseDirectionX*dt;
        glm::quat deltaRot = glm::angleAxis(angle, localUp);
        _rotation = deltaRot * _rotation;
        mouseDeltaX = 0;
    }
        
    if(mouseDeltaY != 0) {
        float mouseDirectionY = -mouseDeltaY*_mouseSensitivity*dt;
        float angle = mouseDirectionY*dt;
        glm::quat deltaRot = glm::angleAxis(angle, localRight);
        _rotation = _rotation * deltaRot;
        mouseDeltaY = 0;
    }
        
    _eye += velocity;
    _center = _eye + worldForward;
    _up = worldUp;
        
    _activeCamera.component<Transform>()->value = glm::lookAt(_eye, _center, _up);
}
    
void CameraMovementSystem::receive(const entityx::ComponentAddedEvent<ActiveCamera> &event)
{
    _activeCamera = event.entity;
    const glm::mat4 &cameraTransform = _activeCamera.component<Transform>()->value;
    _eye = -glm::vec3(cameraTransform[3]);
    _center = _eye + glm::vec3(cameraTransform[2]);
    _up = glm::vec3(cameraTransform[1]);
    _rotation = glm::quat_cast(cameraTransform);
}
    
void CameraMovementSystem::receive(const entityx::ComponentRemovedEvent<ActiveCamera> &event)
{
    if (_activeCamera == event.entity) {
        _activeCamera.invalidate();
    }
}
    
void CameraMovementSystem::receive(const KeypressEvent &event)
{
    _keys[event.key] = event.down;
}
    
void CameraMovementSystem::receive(const MouseMoveEvent &event)
{
    _mouseEventPending = true;
}
