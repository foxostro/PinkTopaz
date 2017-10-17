//
//  MemoryMappedFile.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/8/17.
//
//

#ifndef MemoryMappedFile_hpp
#define MemoryMappedFile_hpp

#include <memory>
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

// Map a file in memory without having to be concerned with platform details.
class MemoryMappedFile
{
public:
    // Destructor.
    ~MemoryMappedFile();
    
    // Constructor. The file won't be mapped until the next call to mapFile().
    MemoryMappedFile(const boost::filesystem::path &regionFileName);
    
    // Map the file. Truncate size to at least the specified size.
    // Returns true if this caused the file to be created.
    bool mapFile(size_t minimumFileSize);
    
    // Unmap the file.
    void unmapFile();
    
    // Get the name of the file to be mapped.
    boost::filesystem::path fileName() const;
    
    // Get a pointer to the mapped region.
    void* mapping();
    
    // Get a pointer to the mapped region.
    const void* mapping() const;
    
    // Get the number of bytes in the mapped region.
    size_t size() const;
    
private:
    boost::filesystem::path _fileName;
    boost::iostreams::mapped_file _fileMapping;
};

#endif /* MemoryMappedFile_hpp */
