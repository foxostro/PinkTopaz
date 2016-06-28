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
        std::ifstream in(filePath, std::ios::in | std::ios::binary);

        if (!in) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "failed to open file: %s, filePath=%s\n", strerror(errno), filePath);
        }

        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();

        return contents;
    }

} // namespace PinkTopaz
