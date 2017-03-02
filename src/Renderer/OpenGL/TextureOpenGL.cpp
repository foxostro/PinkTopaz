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
    
    GLint textureAddressModeEnum(TextureAddressMode mode)
    {
        switch (mode)
        {
            case Repeat: return GL_REPEAT;
            case ClampToEdge: return GL_CLAMP_TO_EDGE;
            default:
                throw Exception("Unsupported texture format.");
        }
    }
    
    GLint textureFilterEnum(TextureFilter filter)
    {
        switch (filter)
        {
            case Nearest: return GL_NEAREST;
            case Linear: return GL_LINEAR;
            case NearestMipMapNearest: return GL_NEAREST_MIPMAP_NEAREST;
            default:
                throw Exception("Unsupported texture format.");
        }
    }
    
    TextureOpenGL::TextureOpenGL(CommandQueue &queue,
                                 const TextureDescriptor &desc,
                                 const void *data)
     : _commandQueue(queue)
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
            
            const GLint addressS = textureAddressModeEnum(desc.addressS);
            const GLint addressT = textureAddressModeEnum(desc.addressT);
            const GLint minFilter = textureFilterEnum(desc.minFilter);
            const GLint maxFilter = textureFilterEnum(desc.maxFilter);
            
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
            
            glTexParameteri(target, GL_TEXTURE_WRAP_S, addressS);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, addressT);
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, maxFilter);
            
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
