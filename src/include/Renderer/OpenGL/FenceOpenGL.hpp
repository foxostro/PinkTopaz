//
//  FenceOpenGL.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/3/17.
//
//

#ifndef FenceOpenGL_hpp
#define FenceOpenGL_hpp

#include "Renderer/Fence.hpp"
#include "Renderer/OpenGL/opengl.hpp"

namespace PinkTopaz::Renderer::OpenGL {
    
    class FenceOpenGL : public Fence
    {
    public:
        FenceOpenGL();
        ~FenceOpenGL();
        inline GLsync getObject() const { return _object; }
        
    private:
        GLsync _object;
    };
    
} // namespace PinkTopaz::Renderer::OpenGL

#endif /* FenceOpenGL_hpp */
