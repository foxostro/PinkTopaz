//
//  String.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 2/28/17.
//
//

#include "Renderer/String.hpp"

namespace PinkTopaz::Renderer {
    
    String::String(const std::string &contents,
                   const glm::vec2 &position,
                   float scale)
     : _contents(contents),
       _position(position),
       _scale(scale)
    {}
    
} // namespace PinkTopaz::Renderer
