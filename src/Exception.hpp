//
//  Exception.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/4/16.
//
//

#ifndef Exception_hpp
#define Exception_hpp

#include <exception>
#include <string>

namespace PinkTopaz {
    
    class Exception : public std::exception
    {
    private:
        std::string _reason;
        
    public:
        Exception(const std::string fmt, ...);
        
        virtual const char *what() const noexcept override
        {
            return _reason.c_str();
        }
    };
    
} // namespace PinkTopaz

#endif /* Exception_hpp */
