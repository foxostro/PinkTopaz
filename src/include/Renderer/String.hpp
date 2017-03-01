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

namespace PinkTopaz::Renderer {
    
    class String
    {
    public:
        String(const std::string &contents,
               const glm::vec2 &position,
               float scale);
        
        ~String() = default;
        
        inline const std::string &getContents() const { return _contents; }
        inline const glm::vec2 &getPosition() const { return _position; }
        inline float getScale() const { return _scale; }
        
    private:
        const std::string _contents;
        const glm::vec2 _position;
        const float _scale;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* String_hpp */
