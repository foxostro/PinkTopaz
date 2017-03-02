//
//  TextureOpenGL.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/26/17.
//
//

#include "Renderer/OpenGL/TextureOpenGL.hpp"
#include "Renderer/OpenGL/glUtilities.hpp"
#include "Exception.hpp"
#include <cassert>

namespace PinkTopaz::Renderer::OpenGL {
    
    GLenum textureTargetEnum(TextureType type)
    {
        switch (type)
        {
            case Texture2D: return GL_TEXTURE_2D;
            case Texture2DArray: return GL_TEXTURE_2D_ARRAY;
            default:
                throw Exception("Unsupported texture type.");
        }
    }
    
    GLint textureExternalFormat(TextureFormat format)
    {
        switch (format)
        {
            case R8: return GL_RED;
            case RGBA8: return GL_RGBA;
            case BGRA8: return GL_BGRA;
            default:
                throw Exception("Unsupported texture type.");
        }
    }
    
    GLint textureInternalFormat(TextureFormat format)
    {
        switch (format)
        {
            case R8: return GL_RED;
            case RGBA8: return GL_RGBA;
            case BGRA8: return GL_RGBA;
            default:
                throw Exception("Unsupported texture type.");
        }
    }
    
    GLint textureDataType(TextureFormat format)
    {
        switch (format)
        {
            case R8: return GL_UNSIGNED_BYTE;
            case RGBA8: return GL_UNSIGNED_BYTE;
            case BGRA8: return GL_UNSIGNED_BYTE;
            default:
                throw Exception("Unsupported texture format.");
        }
    }
    
    size_t textureDataTypeSize(TextureFormat format)
    {
        switch (format)
        {
            case R8: return 1;
            case RGBA8: return 4;
            case BGRA8: return 4;
            default:
                throw Exception("Unsupported texture format.");
        }
    }
    
    TextureOpenGL::TextureOpenGL(CommandQueue &queue,
                                 const TextureDescriptor &desc,
                                 const void *data)
     : _target(0),
       _handle(0),
       _commandQueue(queue)
    {
        const size_t len = desc.width * desc.height * desc.depth * textureDataTypeSize(desc.format);
        std::vector<uint8_t> wrappedData(len);
        memcpy(&wrappedData[0], data, len);
        
        const GLenum target = _target = textureTargetEnum(desc.type);
        constexpr GLint level = 0;
        const GLint internalFormat = textureInternalFormat(desc.format);
        constexpr GLint border = 0;
        const GLint externalFormat = textureExternalFormat(desc.format);
        const GLint dataType = textureDataType(desc.format);
        
        _commandQueue.enqueue([=]{
            const void *bytes = &wrappedData[0];
            
            glPixelStorei(GL_UNPACK_ALIGNMENT, desc.unpackAlignment);

            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(target, texture);
            
            switch (target)
            {
                case GL_TEXTURE_2D:
                    glTexImage2D(target, level, internalFormat, desc.width,
                                 desc.height, border, externalFormat, dataType, bytes);
                    break;
                    
                case GL_TEXTURE_2D_ARRAY:
                    glTexImage3D(target, level, internalFormat, desc.width,
                                 desc.height, desc.depth, border, externalFormat,
                                 dataType, bytes);
                    break;
            }
            
            if (desc.generateMipMaps) {
                glGenerateMipmap(target);
            }
            
            glBindTexture(target, 0);
            CHECK_GL_ERROR();
            
            this->_handle = texture;
        });
    }
    
    TextureOpenGL::~TextureOpenGL()
    {
        GLuint handle = _handle;
        _commandQueue.enqueue([handle]{
            glDeleteTextures(1, &handle);
            CHECK_GL_ERROR();
        });
    }
    
} // namespace PinkTopaz::Renderer::OpenGL