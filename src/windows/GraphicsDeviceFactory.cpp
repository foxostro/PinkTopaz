//
//  GraphicsDeviceFactory.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#include "Renderer/GraphicsDevice.hpp"
#include "Renderer/OpenGL/GraphicsDeviceOpenGL.hpp"
    
std::shared_ptr<GraphicsDevice> createDefaultGraphicsDevice(SDL_Window &w)
{
    auto concrete = std::make_shared<GraphicsDeviceOpenGL>(w);
    auto abstract = std::dynamic_pointer_cast<GraphicsDevice>(concrete);
    return abstract;
}
