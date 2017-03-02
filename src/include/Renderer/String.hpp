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
        String(const std::string &contents, const glm::vec2 &position)
         : _contents(contents),
           _position(position)
        {}
        
        ~String() = default;
        
        inline void setContents(const std::string &s) { _contents = s; }
        inline const std::string &getContents() const { return _contents; }
        inline const glm::vec2 &getPosition() const { return _position; }
        
    private:
        std::string _contents;
        const glm::vec2 _position;
    };
    
} // namespace PinkTopaz::Renderer

#endif /* String_hpp */
