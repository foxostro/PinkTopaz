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
#include <glm/vec4.hpp>

enum TextWeight
{
    Light,
    Regular,
    Bold
};

struct TextAttributes
{
    std::string fontName;
    unsigned fontSize;
    unsigned border;
    glm::vec4 color;
    glm::vec4 borderColor;
    TextWeight weight;
    
    TextAttributes() : fontSize(12), border(0), weight(Regular) {}
    
    TextAttributes(const std::string &myFontName,
                   unsigned myFontSize,
                   unsigned myBorder,
                   const glm::vec4 &myColor,
                   const glm::vec4 &myBorderColor,
                   TextWeight myWeight)
     : fontName(myFontName),
       fontSize(myFontSize),
       border(myBorder),
       color(myColor),
       borderColor(myBorderColor),
       weight(myWeight)
    {}
};

#endif /* TextAttributes_hpp */
