//
//  GraphicsDeviceOpenGL.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/8/16.
//
//

#ifndef GraphicsDeviceOpenGL_hpp
#define GraphicsDeviceOpenGL_hpp

#include <mutex>
#include <queue>
#include <memory>

#include "SDL.h"
#include "Renderer/GraphicsDevice.hpp"
#include "Renderer/CommandEncoder.hpp"
#include "Renderer/OpenGL/CommandQueue.hpp"

namespace PinkTopaz::Renderer::OpenGL {
    
    class GraphicsDeviceOpenGL : public GraphicsDevice
    {
    public:
        GraphicsDeviceOpenGL(SDL_Window &window);
        virtual ~GraphicsDeviceOpenGL();
        
        // Call this to begin a frame. It returns a command encoder which can be
        // used to encoder graphics commands for submission to the graphics
        // at the end of the frame.
        virtual std::shared_ptr<CommandEncoder> encoder() override;
        
        // Call this to submit commands to the graphics device.
        // This call is garaunteed thread-safe. Commands will be submitted to
        // GPU in the order of call to this method; however, commands may not
        // be flushed until the next call to swapBuffers().
        virtual void submit(const std::shared_ptr<CommandEncoder> &encoder) override;
        
        // Flushes commands and swaps buffers. Some underlying graphics APIs
        // have restrictions about which threads they can be used on. So, it's
        // the caller's responsibility call only from method on the main thread.
        virtual void swapBuffers() override;
        
        // Creates a new shader using the specified GLSL source code strings.
        // TODO: Add a way to specify the appropriate shader program in a
        // platform agnostic manner. Maybe use a name for which source can be
        // looked up for GLSL, or HLSL, or Metal, or whatever.
        virtual std::shared_ptr<Shader> makeShader(const std::string &vertexShaderSource, const std::string &fragmentShaderSource) override;
        
        // Creates a new texture array from the specified image file.
        // TODO: I don't think image loading should be in the graphics device.
        virtual std::shared_ptr<TextureArray> makeTextureArray(const char *fileName) override;
        
        // Creates a new GPU buffer object.
        virtual std::shared_ptr<Buffer>
        makeBuffer(const VertexFormat &format,
                   const std::vector<uint8_t> &bufferData,
                   size_t count,
                   BufferUsage usage) override;
        
    private:
        SDL_Window &_window;
        SDL_GLContext _glContext;
        CommandQueue _commandQueue;
    };
    
} // namespace PinkTopaz::Renderer::OpenGL

#endif /* GraphicsDeviceOpenGL_hpp */
