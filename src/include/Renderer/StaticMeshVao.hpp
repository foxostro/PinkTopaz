//
//  StaticMeshVao.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/4/16.
//
//

#ifndef StaticMeshVao_hpp
#define StaticMeshVao_hpp

#include <vector>

namespace PinkTopaz::Renderer {
    
    // Enacpsulates a Vertex Array Object in a platorm-agnostic manner.
    class StaticMeshVao
    {
    public:
        virtual ~StaticMeshVao() = default;
        virtual size_t getNumVerts() const = 0;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* StaticMeshVao_hpp */
