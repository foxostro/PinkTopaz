//
//  Exception.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/4/16.
//
//

#include "Exception.hpp"
#include "SDL.h"

#include <cstdarg>
#include <memory>
#include <vector>
#include <spdlog/spdlog.h>

std::string Exception::formatVarArgs(const std::string fmt_str, va_list arglist) noexcept
{
    // http://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
    int n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
    std::string str;
    std::unique_ptr<char[]> formatted;
    while(1) {
        formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
        strcpy(&formatted[0], fmt_str.c_str());
        va_list ap;
        va_copy(ap, arglist);
        int final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
        va_end(ap);
        if (final_n < 0 || final_n >= n)
            n += abs(final_n - n + 1);
        else
            break;
    }
    
    return formatted.get();
}

void Exception::init(const std::string fmt_str, va_list arglist) noexcept
{
    _reason = formatVarArgs(fmt_str, arglist);
    spdlog::get("console")->error(_reason);
}
    
Exception::Exception(const std::string fmt_str, ...)
{
    va_list arglist;
    va_start(arglist, fmt_str);
    init(fmt_str, arglist);
    va_end(arglist);
}
