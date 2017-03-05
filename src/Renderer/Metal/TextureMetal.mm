//
//  TextureMetal.mm
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#include "Renderer/Metal/TextureMetal.h"
#include "Exception.hpp"

namespace PinkTopaz::Renderer::Metal {

    TextureMetal::TextureMetal(id <MTLDevice> device,
                               const TextureDescriptor &desc,
                               const void *data)
    {
        _texture = nil;
    }
    
    TextureMetal::~TextureMetal()
    {
        [_texture release];
    }

} // namespace PinkTopaz::Renderer::Metal
