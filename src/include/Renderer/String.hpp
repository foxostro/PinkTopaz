//
//  String.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#ifndef String_hpp
#define String_hpp

#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "Renderer/Buffer.hpp"

namespace PinkTopaz::Renderer {
    
    class String
    {
    public:
        String(const std::string &str, const glm::vec2 &p, const glm::vec3 &c)
         : contents(str),
           position(p),
           color(c)
        {}
        
        ~String() = default;
        
        std::string contents;
        glm::vec2 position;
        glm::vec3 color;

        std::shared_ptr<Buffer> buffer;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* String_hpp */
