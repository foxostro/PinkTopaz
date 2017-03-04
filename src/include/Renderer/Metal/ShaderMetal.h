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

namespace PinkTopaz::Renderer::Metal {
    
    class ShaderMetal : public Shader
    {
    public:
        ShaderMetal(const std::string &vert, const std::string &frag);
        virtual ~ShaderMetal();
    };
    
} // namespace PinkTopaz::Renderer::Metal

#endif /* ShaderMetal_h */
