//
//  TextAttributes.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 11/7/17.
//
//

#ifndef TextAttributes_hpp
#define TextAttributes_hpp

#include <boost/filesystem.hpp>
#include <glm/vec2.hpp>

struct TextAttributes
{
    boost::filesystem::path fontName;
    unsigned fontSize;
    unsigned border;
    glm::vec4 color;
    glm::vec4 borderColor;
};

#endif /* TextAttributes_hpp */
