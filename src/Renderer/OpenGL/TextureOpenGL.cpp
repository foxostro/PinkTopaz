//
//  TextureOpenGL.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/26/17.
//
//

#include "Renderer/OpenGL/TextureOpenGL.hpp"
#include "Renderer/OpenGL/glUtilities.hpp"

GLenum textureTargetEnum(TextureType type)
{
    switch (type)
    {
        case Texture2D: return GL_TEXTURE_2D;
        case Texture2DArray: return GL_TEXTURE_2D_ARRAY;
        default:
            throw UnsupportedTextureTypeException(type);
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
            throw UnsupportedTextureFormatException(format);
    }
}

GLint textureInternalFormat(TextureFormat format)
{
    switch (format)
    {
        case R8: return GL_RED;
        case RGBA8: return GL_SRGB_ALPHA;
        case BGRA8: return GL_SRGB_ALPHA;
        default:
            throw UnsupportedTextureFormatException(format);
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
            throw UnsupportedTextureFormatException(format);
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
            throw UnsupportedTextureFormatException(format);
    }
}

TextureOpenGL::TextureOpenGL(unsigned id,
                             const std::shared_ptr<CommandQueue> &commandQueue,
                             const TextureDescriptor &desc,
                             const void *data)
 : _id(id),
   _target(0),
   _handle(0),
   _commandQueue(commandQueue)
{
    const size_t len = desc.width * desc.height * desc.depth * textureDataTypeSize(desc.format);
    std::vector<uint8_t> wrappedData(len);
    memcpy(&wrappedData[0], data, len);
    
    commonInit(desc, wrappedData);
}

TextureOpenGL::TextureOpenGL(unsigned id,
                             const std::shared_ptr<CommandQueue> &commandQueue,
                             const TextureDescriptor &desc,
                             const std::vector<uint8_t> &data)
 : _id(id),
   _target(0),
   _handle(0),
   _commandQueue(commandQueue)
{
    const size_t expectedLen = desc.width * desc.height * desc.depth * textureDataTypeSize(desc.format);
    const size_t dataLen = data.size();
    if (expectedLen != dataLen) {
        throw InvalidTextureDataOpenGLException("`data' is not the right size");
    }
    
    commonInit(desc, data);
}

void TextureOpenGL::commonInit(const TextureDescriptor &desc,
                               const std::vector<uint8_t> &data)
{
    _commandQueue->enqueue(_id, __FUNCTION__, [=]{
        const GLenum target = _target = textureTargetEnum(desc.type);
        constexpr GLint level = 0;
        const GLint internalFormat = textureInternalFormat(desc.format);
        constexpr GLint border = 0;
        const GLint externalFormat = textureExternalFormat(desc.format);
        const GLint dataType = textureDataType(desc.format);
        
        const void *bytes = &data[0];
        
        glPixelStorei(GL_UNPACK_ALIGNMENT, desc.unpackAlignment);
        
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(target, texture);
        
        switch (target)
        {
            case GL_TEXTURE_2D:
                glTexImage2D(target, level, internalFormat, (GLsizei)desc.width,
                             (GLsizei)desc.height, border, externalFormat,
                             dataType, bytes);
                break;
                
            case GL_TEXTURE_2D_ARRAY:
                glTexImage3D(target, level, internalFormat, (GLsizei)desc.width,
                             (GLsizei)desc.height, (GLsizei)desc.depth, border,
                             externalFormat, dataType, bytes);
                break;
        }
        
        if (desc.generateMipMaps) {
            glGenerateMipmap(target);
        }
        
        glBindTexture(target, 0);
        CHECK_GL_ERROR();
    
        _handle = texture;
    });
}

TextureOpenGL::~TextureOpenGL()
{
    const GLuint handle = _handle;
    _commandQueue->cancel(_id);
    _commandQueue->enqueue(0, __FUNCTION__, [handle]{
        if (handle) {
            glDeleteTextures(1, &handle);
            CHECK_GL_ERROR();
        }
    });
}
