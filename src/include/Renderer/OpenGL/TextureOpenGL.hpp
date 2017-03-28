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
#include "Renderer/OpenGL/opengl.hpp"

#include <vector>

namespace Renderer {
    
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
    
    // Encapsulates a single OpenGL texture resource.
    class TextureOpenGL : public Texture
    {
    public:
        // Constructs the texture.
        // desc -- The texture descriptor provides information about the type
        //         and format of the texture. It does include enough information
        //         to determine how many bytes of data are in the texture.
        // data -- A pointer to the bytes of data used in the texture. This is
        //         copied when received to prevent problems should the OpenGL
        //         commands run at a later time or on a different thread.
        TextureOpenGL(const TextureDescriptor &desc,
                      const void *data);
        
        TextureOpenGL(const TextureDescriptor &desc,
                      const std::vector<uint8_t> &data);
        
        virtual ~TextureOpenGL();
        
        inline GLuint getHandle() const { return _handle; }
        inline GLenum getTarget() const { return _target; }
        
    private:
        void commonInit(const TextureDescriptor &desc,
                        const std::vector<uint8_t> &data);
        
        GLenum _target;
        GLuint _handle;
    };
    
} // namespace Renderer

#endif /* TextureOpenGL_hpp */
