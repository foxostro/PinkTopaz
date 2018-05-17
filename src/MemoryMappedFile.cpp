//
//  MemoryMappedFile.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/8/17.
//
//

#include "MemoryMappedFile.hpp"
#include "Exception.hpp"

MemoryMappedFile::~MemoryMappedFile()
{
    unmapFile();
}

MemoryMappedFile::MemoryMappedFile(const boost::filesystem::path &fileName)
 : _fileName(fileName)
{}

bool MemoryMappedFile::mapFile(size_t minimumFileSize)
{
    bool didCreateFile;
    size_t fileSize;
    
    unmapFile();
    
    // Ensure the file exists and has the specified minimum size, or more.
    if (boost::filesystem::exists(_fileName)) {
        didCreateFile = false;
        fileSize = (size_t)boost::filesystem::file_size(_fileName);
    } else {
        didCreateFile = true;
        fileSize = 0;
        {
            boost::filesystem::ofstream s(_fileName);
        }
    }
    
    if (fileSize < minimumFileSize) {
        boost::filesystem::resize_file(_fileName, minimumFileSize);
    }
    
    // Map the file.
    _fileMapping.open(_fileName);
    
    return didCreateFile;
}

void MemoryMappedFile::unmapFile()
{
    _fileMapping.close();
}

boost::filesystem::path MemoryMappedFile::fileName() const
{
    return _fileName;
}

void* MemoryMappedFile::mapping()
{
    return (void *)_fileMapping.data();
}

const void* MemoryMappedFile::mapping() const
{
    return (const void *)_fileMapping.const_data();
}

size_t MemoryMappedFile::size() const
{
    return _fileMapping.size();
}
