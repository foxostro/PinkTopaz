//
//  TextureSamplerOpenGL.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/2/17.
//
//

#ifndef TextureSamplerOpenGL_hpp
#define TextureSamplerOpenGL_hpp

#include "Renderer/TextureSampler.hpp"
#include "Renderer/OpenGL/CommandQueue.hpp"
#include "Renderer/OpenGL/opengl.hpp"

namespace PinkTopaz::Renderer::OpenGL {
    
    // The OpenGL address mode to use when sampling, as implied by `mode'.
    GLint texturSamplerAddressModeEnum(TextureSamplerAddressMode mode);
    
    // The OpenGL texture filter to use when sampling, as implied by `filter'.
    GLint textureSamplerFilterEnum(TextureSamplerFilter filter);
    
    // Encapsulates a single OpenGL texture sampler object.
    class TextureSamplerOpenGL : public TextureSampler
    {
    public:
        TextureSamplerOpenGL(CommandQueue &queue,
                             const TextureSamplerDescriptor &desc);
        
        virtual ~TextureSamplerOpenGL();
        
        inline GLuint getHandle() const { return _handle; }
        
    private:
        std::atomic<GLuint> _handle;
        CommandQueue &_commandQueue;
    };
    
} // namespace PinkTopaz::Renderer::OpenGL

#endif /* TextureSamplerOpenGL_hpp */
