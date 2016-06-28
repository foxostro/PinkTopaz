//
//  fileUtilities.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/28/16.
//
//

#include "fileUtilities.hpp"
#include <fstream>
#include <streambuf>
#include <unistd.h> // for chdir
#include "SDL.h"

namespace PinkTopaz {
    
    void setCurrentWorkingDirectory(const char *path)
    {
        if (::chdir(path) < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "chdir failed: %s, path=%s\n", strerror(errno), path);
        }
    }
    
    std::string stringFromFileContents(const char *filePath)
    {
        std::ifstream stream(filePath);
        std::string str;

        stream.seekg(0, std::ios::end);
        str.reserve(stream.tellg());
        stream.seekg(0, std::ios::beg);

        str.assign((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        return str;
    }

} // namespace PinkTopaz
