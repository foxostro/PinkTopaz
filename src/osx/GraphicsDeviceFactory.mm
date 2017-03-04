//
//  GraphicsDeviceFactory.mm
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#include "Renderer/GraphicsDevice.hpp"
#include "Renderer/OpenGL/GraphicsDeviceOpenGL.hpp"

namespace PinkTopaz::Renderer {
    
    std::shared_ptr<GraphicsDevice> createDefaultGraphicsDevice(SDL_Window &w)
    {
        auto concrete = std::make_shared<OpenGL::GraphicsDeviceOpenGL>(w);
        auto abstract = std::dynamic_pointer_cast<GraphicsDevice>(concrete);
        return abstract;
    }
    
}; // namespace PinkTopaz::Renderer
