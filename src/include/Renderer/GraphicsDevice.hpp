//
//  GraphicsDevice.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/20/17.
//
//

#ifndef GraphicsDevice_hpp
#define GraphicsDevice_hpp

#include <memory>
#include <string>

#include "Renderer/CommandEncoder.hpp"
#include "Renderer/StaticMesh.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/StaticMeshVao.hpp"

namespace PinkTopaz::Renderer {

    // This is a thin abstraction layer over the graphics API. Instantiate a
    // concrete GraphicsDevice sub-class that implements an API such as OpenGL.
    class GraphicsDevice
    {
    public:
        GraphicsDevice() {}
        virtual ~GraphicsDevice() {}
        
        // Call this to begin a frame. It returns a command encoder which can be
        // used to encoder graphics commands for submission to the graphics
        // at the end of the frame.
        virtual std::shared_ptr<CommandEncoder> encoder() = 0;

        // Call this to submit commands to the graphics device.
        // This call is garaunteed thread-safe. Commands will be submitted to
        // GPU in the order of call to this method; however, commands may not
        // be flushed until the next call to swapBuffers().
        virtual void submit(const std::shared_ptr<CommandEncoder> &encoder) = 0;
        
        // Flushes commands and swaps buffers. Some underlying graphics APIs
        // have restrictions about which threads they can be used on. So, it's
        // the caller's responsibility call only from method on the main thread.
        virtual void swapBuffers() = 0;
        
        // Creates a new shader using the specified GLSL source code strings.
        // TODO: Add a way to specify the appropriate shader program in a
        // platform agnostic manner. Maybe use a name for which source can be
        // looked up for GLSL, or HLSL, or Metal, or whatever.
        virtual std::shared_ptr<Shader> makeShader(const std::string &vertexShaderSource, const std::string &fragmentShaderSource) = 0;
        
        // Creates a new texture array from the specified image file.
        // TODO: I don't think image loading should be in the graphics device.
        virtual std::shared_ptr<TextureArray> makeTextureArray(const char *fileName) = 0;
        
        // Creates a new texture array from the specified image file.
        // TODO: I don't know if Vertex Array Object is the right abstraction.
        // Also, probably we should just provide API for creating buffers and
        // filling in buffer data.
        virtual std::shared_ptr<StaticMeshVao> makeStaticMeshVao(const std::shared_ptr<StaticMesh> &mesh) = 0;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* GraphicsDevice_hpp */
