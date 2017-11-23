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
    
class Exception : public std::exception
{
private:
    std::string _reason;
    
protected:
    static std::string formatVarArgs(const std::string fmt_str, va_list ap) noexcept;
    
    void init(const std::string fmt_str, va_list ap) noexcept;
        
public:
    Exception() = default;
    Exception(const std::string format, ...);
    
    virtual const char *what() const noexcept override
    {
        return _reason.c_str();
    }
};

#endif /* Exception_hpp */
