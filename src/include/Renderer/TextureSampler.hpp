//
//  TextureSampler.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/2/17.
//
//

#ifndef TextureSampler_hpp
#define TextureSampler_hpp

namespace PinkTopaz::Renderer {
    
    enum TextureSamplerAddressMode
    {
        Repeat,
        ClampToEdge
    };
    
    enum TextureSamplerFilter
    {
        Nearest,
        Linear,
        NearestMipMapNearest
    };
    
    struct TextureSamplerDescriptor
    {
        TextureSamplerAddressMode addressS, addressT;
        TextureSamplerFilter minFilter, maxFilter;
    };
    
    // Encapsulates a texture sampler resource in a platform-agnostic manner.
    class TextureSampler
    {
    public:
        virtual ~TextureSampler() = default;
        
    protected:
        TextureSampler() = default;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* TextureSampler_hpp */
