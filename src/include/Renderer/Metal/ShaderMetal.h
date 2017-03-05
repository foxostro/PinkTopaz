//
//  ShaderMetal.h
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#ifndef ShaderMetal_h
#define ShaderMetal_h

#ifndef __OBJC__
#error "This is an Objective-C++ header."
#endif

#include "Renderer/Shader.hpp"
#include <Metal/Metal.h>

namespace PinkTopaz::Renderer::Metal {
    
    class ShaderMetal : public Shader
    {
    public:
        ShaderMetal(id <MTLDevice> device,
                    id <MTLLibrary> library,
                    const std::string &vert,
                    const std::string &frag);
        virtual ~ShaderMetal();
        
        inline id <MTLRenderPipelineState> getPipelineState() const { return _pipelineState; }
        
    private:
        id <MTLRenderPipelineState> _pipelineState;
    };
    
} // namespace PinkTopaz::Renderer::Metal

#endif /* ShaderMetal_h */
