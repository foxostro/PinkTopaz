//
//  TextureMetal.h
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#ifndef TextureMetal_h
#define TextureMetal_h

#ifndef __OBJC__
#error "This is an Objective-C++ header."
#endif

#include "Renderer/Texture.hpp"

#include <Metal/Metal.h>

namespace PinkTopaz::Renderer::Metal {
    
    class TextureMetal : public Texture
    {
    public:
        TextureMetal(id <MTLDevice> device,
                     const TextureDescriptor &desc,
                     const void *data);
        virtual ~TextureMetal();
        
        inline id <MTLTexture> getMetalTexture() const { return _texture; }
        
    private:
        id <MTLTexture> _texture;
    };
    
} // namespace PinkTopaz::Renderer::Metal

#endif /* TextureMetal_h */
