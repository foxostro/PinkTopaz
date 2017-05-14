//
//  FileUtilities.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/28/16.
//
//

#ifndef FileUtilities_hpp
#define FileUtilities_hpp

#include <string>
#include <vector>

void setCurrentWorkingDirectory(const char *path);
std::string stringFromFileContents(const char *filePath);
std::vector<uint8_t> binaryFileContents(const char *filePath);

#endif /* FileUtilities_hpp */
