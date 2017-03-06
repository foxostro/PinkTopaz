//
//  CameraMovementSystem.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/5/17.
//
//

#ifndef CameraMovementSystem_hpp
#define CameraMovementSystem_hpp

#include <entityx/entityx.h>

#include "ActiveCamera.hpp"
#include "KeypressEvent.hpp"

namespace PinkTopaz {
    
    class CameraMovementSystem : public entityx::System<CameraMovementSystem>, public entityx::Receiver<CameraMovementSystem>
    {
    public:
        CameraMovementSystem();
        void configure(entityx::EventManager &em) override;
        void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override;
        void receive(const KeypressEvent &event);
        
    private:
        float _cameraSpeed;
        bool _left, _right, _up, _down;
    };

} // namespace PinkTopaz

#endif /* CameraMovementSystem_hpp */
