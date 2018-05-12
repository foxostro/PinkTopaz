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

#include "Exception.hpp"

// Exception thrown on unexpected failure to write a file.
class FileWriteErrorException : public Exception
{
public:
    FileWriteErrorException(boost::filesystem::path path, int errorNumber)
    : Exception("Failed to open file for writing: {}, "
                "filePath=\"{}\"",
                strerror(errorNumber), path.string())
    {}
};

// Exception thrown on unexpected failure to read a file.
class FileReadErrorException : public Exception
{
public:
    FileReadErrorException(boost::filesystem::path path, int errorNumber)
    : Exception("Failed to open file for reading: {}, "
                "filePath=\"{}\"",
                strerror(errorNumber), path.string())
    {}
};

// Gets the contents of the file from disk. Throws on file error.
std::string stringFromFileContents(const boost::filesystem::path &path);

// Gets the contents of the file from disk, if available. Else, returns none.
boost::optional<std::vector<uint8_t>> binaryFileContents(const boost::filesystem::path &path);

// Saves the specified bytes to disk.
void saveBinaryFile(const boost::filesystem::path &path, const std::vector<uint8_t> &bytes);

boost::filesystem::path getPrefPath();

#endif /* FileUtilities_hpp */
