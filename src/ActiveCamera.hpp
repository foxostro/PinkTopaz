//
//  ActiveCamera.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/8/16.
//
//

#ifndef ActiveCamera_hpp
#define ActiveCamera_hpp

namespace PinkTopaz {
    
    // Tags the entity which acts as the camera.
    // We expect there to only be one entity at a time that has the ActiveCamera component. Systems will generally
    // listen for the event where this component is added to an entity and set that entity as the camera.
    struct ActiveCamera
    {
        ActiveCamera() {}
    };
    
} // namespace PinkTopaz

#endif /* ActiveCamera_hpp */
