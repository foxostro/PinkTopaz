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
#define DEVICE_TYPE GraphicsDeviceMetal
#else
#import "Renderer/OpenGL/GraphicsDeviceOpenGL.hpp"
#define DEVICE_TYPE GraphicsDeviceOpenGL
#endif

std::shared_ptr<GraphicsDevice> createDefaultGraphicsDevice(std::shared_ptr<spdlog::logger> log,
                                                            SDL_Window &w)
{
    auto concrete = std::make_shared<DEVICE_TYPE>(log, w);
    log->info("Created graphics device: {}", concrete->getName().c_str());
    auto abstract = std::dynamic_pointer_cast<GraphicsDevice>(concrete);
    return abstract;
}
