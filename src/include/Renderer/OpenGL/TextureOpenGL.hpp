//
//  TextureOpenGL.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/26/17.
//
//

#ifndef TextureOpenGL_hpp
#define TextureOpenGL_hpp

#include "Renderer/Texture.hpp"
#include "Renderer/OpenGL/CommandQueue.hpp"
#include "Renderer/OpenGL/opengl.hpp"

namespace PinkTopaz::Renderer::OpenGL {
    
    // The OpenGL texture target implied by the texture type in `type'.
    GLenum textureTargetEnum(TextureType type);
    
    // The OpenGL external texture format implied by `format'. This is the
    // format for data being passed into OpenGL.
    GLint textureExternalFormat(TextureFormat format);
    
    // The OpenGL texture's internal format implied by `format'. This may be
    // different than the external format given by `textureFormat()'. This is
    // the format used by OpenGL to store the texture internally.
    GLint textureInternalFormat(TextureFormat format);
    
    // The type for components in the texture, as implied by `format'.
    // For example, some formats imply that each component is an unsigned byte.
    GLint textureDataType(TextureFormat format);
    
    // The size of each texel, as implied by `format'.
    // For example, RGBA8 implies that there are four components, Red, Green,
    // Blue, and Alpha. Each component is eight bits. So, each texel works out
    // to be four bytes.
    size_t textureDataTypeSize(TextureFormat format);
    
    // The OpenGL address mode to use when sampling, as implied by `mode'.
    GLint textureWrapModeEnum(TextureWrapMode mode);
    
    // The OpenGL texture filter to use when sampling, as implied by `filter'.
    GLint textureFilterEnum(TextureFilter filter);
    
    // Encapsulates a single OpenGL texture resource.
    class TextureOpenGL : public Texture
    {
    public:
        // Constructs the texture.
        // queue -- The command queue is used so that OpenGL commands can be
        //          issued on the blessed OpenGL thread in a thread-safe manner.
        // desc -- The texture descriptor provides information about the type
        //         and format of the texture. It does include enough information
        //         to determine how many bytes of data are in the texture.
        // data -- A pointer to the bytes of data used in the texture. This is
        //         copied when received to prevent problems should the OpenGL
        //         commands run at a later time or on a different thread.
        TextureOpenGL(CommandQueue &queue,
                      const TextureDescriptor &desc,
                      const void *data);
        
        virtual ~TextureOpenGL();
        
        inline GLuint getHandle() const { return _handle; }
        inline GLenum getTarget() const { return _target; }
        
    private:
        GLenum _target;
        std::atomic<GLuint> _handle;
        CommandQueue &_commandQueue;
    };
    
} // namespace PinkTopaz::Renderer::OpenGL

#endif /* TextureOpenGL_hpp */
