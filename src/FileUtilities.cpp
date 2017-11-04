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

std::string stringFromFileContents(const std::string &filePathStr)
{
    const char *filePath = filePathStr.c_str();
    
    std::ifstream in(filePath, std::ios::in | std::ios::binary);
    
    if (!in) {
        throw Exception("Failed to open file: %s, filePath=\"%s\"\n", strerror(errno), filePath);
    }
    
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    
    return contents;
}

boost::optional<std::vector<uint8_t>>
binaryFileContents(const std::string &filePathStr)
{
	const char *filePath = filePathStr.c_str();
    
    std::ifstream in(filePath, std::ios::in | std::ios::binary);
    
    if (!in) {
        return boost::none;
    }
    
    std::vector<uint8_t> contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read((char *)contents.data(), contents.size());
    in.close();
        
    return boost::make_optional(std::move(contents));
}

void saveBinaryFile(const std::string &filePathStr, const std::vector<uint8_t> &bytes)
{
	const char *filePath = filePathStr.c_str();
    
    std::ofstream out(filePath, std::ios::out | std::ios::binary);
    
    if (!out) {
        throw Exception("Failed to save file: %s, filePath=%s\n", strerror(errno), filePath);
    }
    
    out.write((const char *)bytes.data(), bytes.size());
    out.close();
}

std::string getPrefPath()
{
    char *s = SDL_GetPrefPath("foxostro", "PinkTopaz");
    std::string prefPath(s);
    SDL_free(s);
    return prefPath;
}
