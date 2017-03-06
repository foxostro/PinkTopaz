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

#include "SDL.h"

namespace PinkTopaz {
    
    CameraMovementSystem::CameraMovementSystem()
     : _cameraSpeed(1.0), _left(false), _right(false), _up(false), _down(false)
    {}
    
    void CameraMovementSystem::configure(entityx::EventManager &em)
    {
        em.subscribe<KeypressEvent>(*this);
    }
    
    void CameraMovementSystem::update(entityx::EntityManager &es,
                                      entityx::EventManager &events,
                                      entityx::TimeDelta dt)
    {
        auto f = [&](entityx::Entity entity,
                     ActiveCamera &activeCamera,
                     Transform &xform) {
            
            const float angle = _cameraSpeed*dt/1000.0;
            
            static const glm::vec3 right(1, 0, 0);
            static const glm::vec3 up(0, 1, 0);
            
            if(_up) {
                xform.rotation = glm::angleAxis(-angle, right) * xform.rotation;
            } else if(_down) {
                xform.rotation = glm::angleAxis(angle, right) * xform.rotation;
            }
            
            if(_left) {
                xform.rotation *= glm::angleAxis(-angle, up);
            } else if(_right) {
                xform.rotation *= glm::angleAxis(angle, up);
            }
        };
        es.each<ActiveCamera, Transform>(f);
    }
    
    void CameraMovementSystem::receive(const KeypressEvent &event)
    {
        bool state = event.down;
        
        switch (event.key)
        {
            case SDLK_LEFT:     _left   = state; break;
            case SDLK_RIGHT:    _right  = state; break;
            case SDLK_UP:       _up     = state; break;
            case SDLK_DOWN:     _down   = state; break;
        }
    }
    
} // namespace PinkTopaz
