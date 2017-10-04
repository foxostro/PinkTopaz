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

std::string stringFromFileContents(const boost::filesystem::path &path)
{
	const std::string filePathStr = path.string();
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
binaryFileContents(const boost::filesystem::path &path)
{
	const std::string filePathStr = path.string();
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

void saveBinaryFile(const boost::filesystem::path &path, const std::vector<uint8_t> &bytes)
{
	const std::string filePathStr = path.string();
	const char *filePath = filePathStr.c_str();
    
    std::ofstream out(filePath, std::ios::out | std::ios::binary);
    
    if (!out) {
        throw Exception("Failed to save file: %s, filePath=%s\n", strerror(errno), filePath);
    }
    
    out.write((const char *)bytes.data(), bytes.size());
    out.close();
}

boost::filesystem::path getPrefPath()
{
    char *s = SDL_GetPrefPath("foxostro", "PinkTopaz");
    boost::filesystem::path prefPath(s);
    SDL_free(s);
    return prefPath;
}
