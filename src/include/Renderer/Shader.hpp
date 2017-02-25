//
//  Shader.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/28/16.
//
//

#ifndef Shader_hpp
#define Shader_hpp

#include <glm/mat4x4.hpp>

namespace PinkTopaz::Renderer {
    
    // Encapsulates a Shader resource in a platform-agnostic manner.
    class Shader
    {
    public:
        virtual ~Shader();
        
        // TODO: Use uniform buffer objects instead.
        virtual void setShaderUniform(const char *name, const glm::mat4 &value) = 0;
        virtual void setShaderUniform(const char *name, int value) = 0;
        
    protected:
        Shader();
    };

} // namespace PinkTopaz::Renderer

#endif /* Shader_hpp */
