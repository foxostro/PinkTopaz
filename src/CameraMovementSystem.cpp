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
   _mouseSensitivity(5.0f)
{}
    
void CameraMovementSystem::configure(entityx::EventManager &em)
{
    em.subscribe<entityx::ComponentAddedEvent<ActiveCamera>>(*this);
    em.subscribe<entityx::ComponentRemovedEvent<ActiveCamera>>(*this);
    
    // Get the initial delta position to reset.
    int mouseDeltaX = 0, mouseDeltaY = 0;
    SDL_GetRelativeMouseState(&mouseDeltaX, &mouseDeltaY);
}
    
void CameraMovementSystem::update(entityx::EntityManager &es,
                                    entityx::EventManager &events,
                                    entityx::TimeDelta deltaMilliseconds)
{
    const entityx::TimeDelta dt = deltaMilliseconds / 1000;
        
    if (!_activeCamera.valid()) {
        return;
    }
    
    // Poll the keyboard state.
    const Uint8 *keyState = SDL_GetKeyboardState(NULL);
    
    // Poll the mouse position once per frame. We measure the delta of the mouse
    // position and use that to control the camera.
    int mouseDeltaX = 0, mouseDeltaY = 0;
    SDL_GetRelativeMouseState(&mouseDeltaX, &mouseDeltaY);
    
    const glm::vec3 localForward(0, 0, -1);
    const glm::vec3 localRight(1, 0, 0);
    const glm::vec3 localUp(0, 1, 0);
        
    const glm::vec3 worldForward = glm::normalize(_rotation * localForward);
    const glm::vec3 worldRight = glm::normalize(_rotation * localRight);
    const glm::vec3 worldUp = glm::normalize(_rotation * localUp);
        
    glm::vec3 velocity;
        
    const float angle = _cameraRotateSpeed*dt;
    const float speed = _cameraSpeed*dt;
        
    if (keyState[SDL_SCANCODE_W]) {
        velocity += worldForward * speed;
    } else if(keyState[SDL_SCANCODE_S]) {
        velocity += worldForward * -speed;
    }
        
    if (keyState[SDL_SCANCODE_A]) {
        velocity += worldRight * -speed;
    } else if(keyState[SDL_SCANCODE_D]) {
        velocity += worldRight * speed;
    }
        
    if (keyState[SDL_SCANCODE_Z]) {
        velocity += localUp * -speed;
    } else if(keyState[SDL_SCANCODE_X]) {
        velocity += localUp * speed;
    }

    if (keyState[SDL_SCANCODE_I]) {
        _rotation = glm::angleAxis(-angle, worldRight) * _rotation;
    } else if(keyState[SDL_SCANCODE_K]) {
        _rotation = glm::angleAxis(angle, worldRight) * _rotation;
    }
        
    if (keyState[SDL_SCANCODE_J]) {
        glm::quat deltaRot = glm::angleAxis(angle, localUp);
        _rotation = deltaRot * _rotation;
    } else if(keyState[SDL_SCANCODE_L]) {
        glm::quat deltaRot = glm::angleAxis(-angle, localUp);
        _rotation = deltaRot * _rotation;
    }
    
    if (mouseDeltaX != 0) {
        float mouseDirectionX = -mouseDeltaX*_mouseSensitivity*dt;
        float angle = mouseDirectionX*dt;
        glm::quat deltaRot = glm::angleAxis(angle, localUp);
        _rotation = deltaRot * _rotation;
        mouseDeltaX = 0;
    }
        
    if (mouseDeltaY != 0) {
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
