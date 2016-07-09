//
//  World.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/8/16.
//
//

#ifndef World_hpp
#define World_hpp

#include <entityx/entityx.h>
#include "StaticMesh.hpp"
#include "Shader.hpp"
#include "TextureArray.hpp"

namespace PinkTopaz {
    
    struct WindowSizeChangedEvent
    {
        WindowSizeChangedEvent() : windowScaleFactor(0), width(0), height(0) {}

        float windowScaleFactor;
        int width, height;
    };
    
    // A World is the same thing as a game zone or level.
    // This is a collection of interacting entities and associated systems.
    // It is, of course, entirely possible to have multiple worlds. However, interactions across worlds are not
    // intended to be routine or easily modeled.
    class World : public entityx::EntityX
    {
    public:
        explicit World(const std::shared_ptr<StaticMeshVAO> &vao,
                       const std::shared_ptr<Shader> &shader,
                       const std::shared_ptr<TextureArray> &texture);
        
        void update(entityx::TimeDelta dt);
    };
    
} // namespace PinkTopaz

#endif /* World_hpp */
