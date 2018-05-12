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
#include <fmt/format.h>
    
class Exception : public std::exception
{
private:
    std::string _reason;
    
public:
    Exception() = default;
    Exception(const std::string &reason) : _reason(reason) {}
    
    template<typename... Args>
    Exception(Args&&... args)
    : _reason(fmt::format(std::forward<Args>(args)...))
    {}
    
    virtual const char *what() const noexcept override
    {
        return _reason.c_str();
    }
};

#endif /* Exception_hpp */
