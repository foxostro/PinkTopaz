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
#include <functional>

namespace PinkTopaz::Renderer {
    
    // Encapsulates a Shader resource in a platform-agnostic manner.
    class Shader
    {
    public:
        virtual ~Shader() = default;
        
        // TODO: Use uniform buffer objects instead.
        virtual void setShaderUniform(const char *name, const glm::mat4 &value) = 0;
        virtual void setShaderUniform(const char *name, const glm::vec3 &value) = 0;
        virtual void setShaderUniform(const char *name, int value) = 0;
        
    protected:
        Shader() = default;
    };

} // namespace PinkTopaz::Renderer

#endif /* Shader_hpp */
