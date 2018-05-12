//
//  FreeTypeException.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/12/18.
//
//

#ifndef FreeTypeException_hpp
#define FreeTypeException_hpp

#include "Exception.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
    
// Exception thrown when an error occurs in the FreeType library.
class FreeTypeException : public Exception
{
public:
    template<typename... Args>
    FreeTypeException(FT_Error err, Args&&... args)
    : Exception("{}: {}",
                fmt::format(std::forward<Args>(args)...),
                errorString(err))
    {}
    
    static const char * errorString(FT_Error err)
    {
#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  case e: return s;
#define FT_ERROR_START_LIST     switch (err) {
#define FT_ERROR_END_LIST       }
#include FT_ERRORS_H
        return "Unknown Error";
    }
};

#endif /* FreeTypeException_hpp */
