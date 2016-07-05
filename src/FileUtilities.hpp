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

namespace PinkTopaz {
    
    void setCurrentWorkingDirectory(const char *path);
    std::string stringFromFileContents(const char *filePath);
    std::vector<uint8_t> binaryFileContents(const char *filePath);

} // namespace PinkTopaz

#endif /* FileUtilities_hpp */
