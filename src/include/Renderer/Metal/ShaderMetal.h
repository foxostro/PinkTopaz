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
#include "Renderer/VertexFormat.hpp"
#include <Metal/Metal.h>

class ShaderMetal : public Shader
{
public:
    ShaderMetal(const VertexFormat &_vertexFormat,
                id <MTLDevice> device,
                id <MTLLibrary> library,
                const std::string &vert,
                const std::string &frag,
                bool blending);
    
    virtual ~ShaderMetal();
    
    inline id <MTLRenderPipelineState> getPipelineState() const { return _pipelineState; }
    inline bool getBlending() const { return _blending; }
    
private:
    id <MTLRenderPipelineState> _pipelineState;
    bool _blending;
};

#endif /* ShaderMetal_h */
