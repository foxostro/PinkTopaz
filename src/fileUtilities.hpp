//
//  fileUtilities.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 6/28/16.
//
//

#ifndef fileUtilities_hpp
#define fileUtilities_hpp

#include <string>

namespace PinkTopaz {
    
    void setCurrentWorkingDirectory(const char *path);
    std::string stringFromFileContents(const char *filePath);

} // namespace PinkTopaz

#endif /* fileUtilities_hpp */
