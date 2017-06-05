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

std::string stringFromFileContents(const boost::filesystem::path &path);
std::vector<uint8_t> binaryFileContents(const boost::filesystem::path &path);
void saveBinaryFile(const boost::filesystem::path &path, const std::vector<uint8_t> &bytes);

#endif /* FileUtilities_hpp */
