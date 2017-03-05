//
//  FenceOpenGL.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/3/17.
//
//

#include "Renderer/OpenGL/FenceOpenGL.hpp"
#include "Renderer/OpenGL/glUtilities.hpp"
#include "Renderer/OpenGL/opengl.hpp"

namespace PinkTopaz::Renderer::OpenGL {
    
    FenceOpenGL::FenceOpenGL()
    {
        constexpr GLenum op = 0x9117;//GL_SYNC_GPU_COMMANDS_COMPLETE​;
        GLsync sync = glFenceSync(op, 0);
        CHECK_GL_ERROR();
        _object = sync;
    }
    
    FenceOpenGL::~FenceOpenGL()
    {
        GLsync object = _object;
        glDeleteSync(object);
        CHECK_GL_ERROR();
    }
    
} // namespace PinkTopaz::Renderer::OpenGL
