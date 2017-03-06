//
//  GraphicsDeviceFactory.mm
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#import "Renderer/GraphicsDevice.hpp"

#ifdef METAL_ENABLED
#import "Renderer/Metal/GraphicsDeviceMetal.h"
#define DEVICE_TYPE Metal::GraphicsDeviceMetal
#else
#import "Renderer/OpenGL/GraphicsDeviceOpenGL.hpp"
#define DEVICE_TYPE OpenGL::GraphicsDeviceOpenGL
#endif

namespace PinkTopaz::Renderer {
    
    std::shared_ptr<GraphicsDevice> createDefaultGraphicsDevice(SDL_Window &w)
    {
        auto concrete = std::make_shared<DEVICE_TYPE>(w);
        auto abstract = std::dynamic_pointer_cast<GraphicsDevice>(concrete);
        return abstract;
    }
    
} // namespace PinkTopaz::Renderer
