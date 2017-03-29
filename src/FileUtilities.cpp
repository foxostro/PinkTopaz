//
//  fileUtilities.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/28/16.
//
//

#include "FileUtilities.hpp"
#include "Exception.hpp"
#include <fstream>
#include <streambuf>
#include "SDL.h"

#ifdef _WIN32
#include <windows.h>

void setCurrentWorkingDirectory(const char *path)
{
    if (!SetCurrentDirectoryA(path)) {
        throw Exception("SetCurrentDirectory failed.\n");
    }
}
#else
#include <unistd.h> // for chdir

void setCurrentWorkingDirectory(const char *path)
{
    if (::chdir(path) < 0) {
        throw Exception("chdir failed: %s, path=%s\n", strerror(errno), path);
    }
}
#endif

std::string stringFromFileContents(const char *filePath)
{
    std::ifstream in(filePath, std::ios::in | std::ios::binary);

    if (!in) {
        throw Exception("Failed to open file: %s, filePath=%s\n", strerror(errno), filePath);
    }

    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();

    return contents;
}
    
std::vector<uint8_t> binaryFileContents(const char *filePath)
{
    std::ifstream in(filePath, std::ios::in | std::ios::binary);
        
    if (!in) {
        throw Exception("Failed to open file: %s, filePath=%s\n", strerror(errno), filePath);
    }
        
    std::vector<uint8_t> contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read((char *)contents.data(), contents.size());
    in.close();
        
    return contents;
}
