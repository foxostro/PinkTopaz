//
//  MemoryMappedFile.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/8/17.
//
//

#ifndef MemoryMappedFile_hpp
#define MemoryMappedFile_hpp

#include <boost/filesystem.hpp>

// Map a file in memory without having to be concerned with platform details.
//
// We don't want to use mapped files from boost::iostreams because we need to
// be able to truncate files too.
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
    
    inline boost::filesystem::path fileName() const
    {
        return _fileName;
    }
    
    inline void* mapping()
    {
        return _mapping;
    }
    
    inline const void* mapping() const
    {
        return _mapping;
    }
    
    inline size_t size() const
    {
        return _size;
    }
    
private:
    boost::filesystem::path _fileName;
    int _fd;
    void *_mapping;
    size_t _size;
};

#endif /* MemoryMappedFile_hpp */
