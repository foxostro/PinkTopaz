//
//  Fence.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 3/3/17.
//
//

#ifndef Fence_hpp
#define Fence_hpp

namespace PinkTopaz::Renderer {
    
    class Fence
    {
    public:
        virtual ~Fence() = default;
        
    protected:
        Fence() = default;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* Fence_hpp */
