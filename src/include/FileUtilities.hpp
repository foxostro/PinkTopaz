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
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

// Gets the contents of the file from disk. Throws on file error.
std::string stringFromFileContents(const std::string &path);

// Gets the contents of the file from disk, if available. Else, returns none.
boost::optional<std::vector<uint8_t>> binaryFileContents(const std::string &path);

// Saves the specified bytes to disk.
void saveBinaryFile(const std::string &path, const std::vector<uint8_t> &bytes);

std::string getPrefPath();

#endif /* FileUtilities_hpp */
