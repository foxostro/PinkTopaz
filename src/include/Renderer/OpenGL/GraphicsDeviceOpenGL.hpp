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
        virtual std::shared_ptr<CommandEncoder> encoder(const RenderPassDescriptor &desc) override;
        
        // Call this to submit commands to the graphics device.
        // This call is garaunteed thread-safe. Commands will be submitted to
        // GPU in the order of call to this method; however, commands may not
        // be flushed until the next call to swapBuffers().
        virtual void submit(const std::shared_ptr<CommandEncoder> &encoder) override;
        
        // Flushes commands and swaps buffers. Some underlying graphics APIs
        // have restrictions about which threads they can be used on. So, it's
        // the caller's responsibility call only from method on the main thread.
        virtual void swapBuffers() override;
        
        // Create a new shader using the specified vertex and fragment programs.
        virtual std::shared_ptr<Shader> makeShader(const std::string &vertexProgramName, const std::string &fragmentProgramName) override;
        
        // Creates a new texture array from the specified image file.
        virtual std::shared_ptr<TextureArray> makeTextureArray(const char *fileName) override;
        
        // Creates a new texture from the specified descriptor and data.
        virtual std::shared_ptr<Texture>
        makeTexture(const TextureDescriptor &desc,
                    const void *data) override;
        
        // Creates a new GPU buffer object.
        virtual std::shared_ptr<Buffer>
        makeBuffer(const VertexFormat &format,
                   const std::vector<uint8_t> &bufferData,
                   size_t elementCount,
                   BufferUsage usage) override;
        
        // Creates a new GPU buffer object with undefined contents.
        virtual std::shared_ptr<Buffer>
        makeBuffer(const VertexFormat &format,
                   size_t size,
                   size_t elementCount,
                   BufferUsage usage) override;
        
    private:
        SDL_Window &_window;
        SDL_GLContext _glContext;
        std::shared_ptr<CommandQueue> _commandQueue;
    };
    
} // namespace PinkTopaz::Renderer::OpenGL

#endif /* GraphicsDeviceOpenGL_hpp */
