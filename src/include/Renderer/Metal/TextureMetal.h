//
//  TextureMetal.h
//  PinkTopaz
//
//  Created by Andrew Fox on 3/4/17.
//
//

#ifndef TextureMetal_hpp
#define TextureMetal_hpp

#ifndef __OBJC__
#error "This is an Objective-C++ header."
#endif

#include "Renderer/Texture.hpp"

#include <vector>

namespace PinkTopaz::Renderer::Metal {
    
    class TextureMetal : public Texture
    {
    public:
        TextureMetal(const TextureDescriptor &desc, const void *data);
        TextureMetal(const TextureDescriptor &desc, const std::vector<uint8_t> &data);
        virtual ~TextureMetal();
    };
    
} // namespace PinkTopaz::Renderer::Metal

#endif /* TextureMetal_hpp */
