//
//  StringRenderer.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#ifndef CerealGLM_hpp
#define CerealGLM_hpp

#include <cereal/archives/binary.hpp>
#include <glm/vec2.hpp>

namespace glm
{
    template<typename Archive>
    void serialize(Archive &archive, glm::vec2 &v)
    {
        archive(cereal::make_nvp("x", v.x),
                cereal::make_nvp("y", v.y));
    }
    
    template<typename Archive>
    void serialize(Archive &archive, glm::ivec2 &v)
    {
        archive(cereal::make_nvp("x", v.x),
                cereal::make_nvp("y", v.y));
    }
}

#endif /* CerealGLM_hpp */
