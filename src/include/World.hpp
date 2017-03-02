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
#include "Renderer/GraphicsDevice.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/TextureArray.hpp"

namespace PinkTopaz {
        
    // A World is the same thing as a game zone or level.
    // This is a collection of interacting entities and associated systems.
    // It is, of course, entirely possible to have multiple worlds. However, interactions across worlds are not
    // intended to be routine or easily modeled.
    class World : public entityx::EntityX
    {
    public:
        explicit World(const std::shared_ptr<Renderer::GraphicsDevice> &renderer,
                       const std::shared_ptr<Renderer::Buffer> &buffer,
                       const std::shared_ptr<Renderer::Shader> &shader,
                       const std::shared_ptr<Renderer::Texture> &texture);
        
        void update(entityx::TimeDelta dt);
    };
    
} // namespace PinkTopaz

#endif /* World_hpp */
