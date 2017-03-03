//
//  TextureSamplerOpenGL.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/2/17.
//
//

#include "Renderer/OpenGL/TextureSamplerOpenGL.hpp"
#include "Renderer/OpenGL/glUtilities.hpp"
#include "Exception.hpp"

namespace PinkTopaz::Renderer::OpenGL {
    
    GLint textureSamplerAddressModeEnum(TextureSamplerAddressMode mode)
    {
        switch (mode)
        {
            case Repeat: return GL_REPEAT;
            case ClampToEdge: return GL_CLAMP_TO_EDGE;
            default:
                throw Exception("Unsupported texture format.");
        }
    }
    
    GLint textureSamplerFilterEnum(TextureSamplerFilter filter)
    {
        switch (filter)
        {
            case Nearest: return GL_NEAREST;
            case Linear: return GL_LINEAR;
            case NearestMipMapNearest: return GL_NEAREST_MIPMAP_NEAREST;
            case LinearMipMapNearest: return GL_LINEAR_MIPMAP_NEAREST;
            case LinearMipMapLinear: return GL_LINEAR_MIPMAP_LINEAR;
            default:
                throw Exception("Unsupported texture format.");
        }
    }
    
    TextureSamplerOpenGL::TextureSamplerOpenGL(CommandQueue &queue, const TextureSamplerDescriptor &desc)
     : _handle(0), _commandQueue(queue)
    {
        const GLint addressS = textureSamplerAddressModeEnum(desc.addressS);
        const GLint addressT = textureSamplerAddressModeEnum(desc.addressT);
        const GLint minFilter = textureSamplerFilterEnum(desc.minFilter);
        const GLint maxFilter = textureSamplerFilterEnum(desc.maxFilter);
        
        _commandQueue.enqueue([=]{
            GLuint sampler;
            glGenSamplers(1, &sampler);
            glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, addressS);
            glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, addressT);
            glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, minFilter);
            glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, maxFilter);
            CHECK_GL_ERROR();
            
            this->_handle = sampler;
        });
    }
    
    TextureSamplerOpenGL::~TextureSamplerOpenGL()
    {
        GLuint handle = _handle;
        _commandQueue.enqueue([handle]{
            glDeleteSamplers(1, &handle);
            CHECK_GL_ERROR();
        });
    }
    
} // namespace PinkTopaz::Renderer::OpenGL
