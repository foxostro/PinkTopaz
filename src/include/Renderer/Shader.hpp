//
//  Shader.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/28/16.
//
//

#ifndef Shader_hpp
#define Shader_hpp

// Encapsulates a Shader resource in a platform-agnostic manner.
class Shader
{
public:
    virtual ~Shader() = default;
    
protected:
    Shader() = default;
};

#endif /* Shader_hpp */
