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
#include <boost/functional/hash.hpp>
#include <glm/vec4.hpp>

enum TextWeight
{
    Light = 0,
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

namespace std {
    template <> struct hash<TextAttributes>
    {
        size_t operator()(const TextAttributes &attr) const
        {
            size_t seed = 0;
            boost::hash_combine(seed, attr.fontName);
            boost::hash_combine(seed, attr.fontSize);
            boost::hash_combine(seed, attr.border);
            boost::hash_combine(seed, attr.color.r);
            boost::hash_combine(seed, attr.color.g);
            boost::hash_combine(seed, attr.color.b);
            boost::hash_combine(seed, attr.color.a);
            boost::hash_combine(seed, attr.borderColor.r);
            boost::hash_combine(seed, attr.borderColor.g);
            boost::hash_combine(seed, attr.borderColor.b);
            boost::hash_combine(seed, attr.borderColor.a);
            boost::hash_combine(seed, (int)attr.weight);
            return seed;
        }
    };
}

#endif /* TextAttributes_hpp */
