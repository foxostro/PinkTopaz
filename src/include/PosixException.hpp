//
//  PosixException.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/12/18.
//
//

#ifndef PosixException_hpp
#define PosixException_hpp

#include "Exception.hpp"
    
// Exception thrown when an error occurs in the POSIX layer.
class PosixException : public Exception
{
public:
    template<typename... Args>
    PosixException(Args&&... args)
    : Exception(std::forward<Args>(args)...)
    {}
};

#endif /* PosixException_hpp */
