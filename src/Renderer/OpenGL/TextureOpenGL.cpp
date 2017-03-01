//
//  TextureOpenGL.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/26/17.
//
//

#include "Renderer/OpenGL/TextureOpenGL.hpp"
#include "Renderer/OpenGL/glUtilities.hpp"
#include <cassert>

namespace PinkTopaz::Renderer::OpenGL {
    
    TextureOpenGL::TextureOpenGL(CommandQueue &queue,
                                 const TextureDescriptor &desc,
                                 const void *data)
     : _commandQueue(queue)
    {
        assert(desc.format == Red); // only one supported right now
        assert(desc.type == Texture2D); // only one supported right now
        
        const size_t len = desc.width * desc.height;
        std::vector<uint8_t> wrappedData(len);
        memcpy(&wrappedData[0], data, len);
        
        _commandQueue.enqueue([this, desc, wrappedData]{
            glPixelStorei(GL_UNPACK_ALIGNMENT, desc.unpackAlignment);

            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         GL_RED,
                         desc.width,
                         desc.height,
                         0,
                         GL_RED,
                         GL_UNSIGNED_BYTE,
                         &wrappedData[0]);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            
            glBindTexture(GL_TEXTURE_2D, 0);
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
