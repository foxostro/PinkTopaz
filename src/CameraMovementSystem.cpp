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
#include "CameraMovedEvent.hpp"

#include <glm/gtc/matrix_transform.hpp>

CameraMovementSystem::CameraMovementSystem()
 : _cameraSpeed(50.0f),
   _cameraRotateSpeed(1.0f),
   _mouseSensitivity(5.0f),
   _mousePosDelta(0, 0)
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
        
    if (_keys[SDLK_w]) {
        velocity += worldForward * speed;
    } else if(_keys[SDLK_s]) {
        velocity += worldForward * -speed;
    }
        
    if (_keys[SDLK_a]) {
        velocity += worldRight * -speed;
    } else if(_keys[SDLK_d]) {
        velocity += worldRight * speed;
    }
        
    if (_keys[SDLK_z]) {
        velocity += localUp * -speed;
    } else if(_keys[SDLK_x]) {
        velocity += localUp * speed;
    }

    if (_keys[SDLK_i]) {
        _rotation = glm::angleAxis(-angle, worldRight) * _rotation;
    } else if(_keys[SDLK_k]) {
        _rotation = glm::angleAxis(angle, worldRight) * _rotation;
    }
        
    if (_keys[SDLK_j]) {
        glm::quat deltaRot = glm::angleAxis(angle, localUp);
        _rotation = deltaRot * _rotation;
    } else if(_keys[SDLK_l]) {
        glm::quat deltaRot = glm::angleAxis(-angle, localUp);
        _rotation = deltaRot * _rotation;
    }
    
    if (_mousePosDelta.x != 0) {
        float mouseDirectionX = -_mousePosDelta.x*_mouseSensitivity*dt;
        float angle = mouseDirectionX*dt;
        glm::quat deltaRot = glm::angleAxis(angle, localUp);
        _rotation = deltaRot * _rotation;
        _mousePosDelta.x = 0;
    }
        
    if (_mousePosDelta.y != 0) {
        float mouseDirectionY = -_mousePosDelta.y*_mouseSensitivity*dt;
        float angle = mouseDirectionY*dt;
        glm::quat deltaRot = glm::angleAxis(angle, localRight);
        _rotation = _rotation * deltaRot;
        _mousePosDelta.y = 0;
    }
        
    _eye += velocity;
    _center = _eye + worldForward;
    _up = worldUp;
    
    glm::mat4 &currentTransform = _activeCamera.component<Transform>()->value;
    const glm::mat4 updatedTransform = glm::lookAt(_eye, _center, _up);
    
    if (currentTransform != updatedTransform) {
        currentTransform = updatedTransform;
        events.emit(CameraMovedEvent());
    }
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
    _mousePosDelta.x = event.deltaX;
    _mousePosDelta.y = event.deltaY;
}
