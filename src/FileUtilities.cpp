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

namespace PinkTopaz {
    
    std::string stringFromFileContents(const boost::filesystem::path &path)
    {
        const char *filePath = path.c_str();
        
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
    
    std::vector<uint8_t> binaryFileContents(const boost::filesystem::path &path)
    {
        const char *filePath = path.c_str();
        
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

} // namespace PinkTopaz
