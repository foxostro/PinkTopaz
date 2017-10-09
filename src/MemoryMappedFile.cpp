//
//  MemoryMappedFile.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/8/17.
//
//

#include "MemoryMappedFile.hpp"
#include "Exception.hpp"
#include "SDL.h" // for SDL_Log
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

MemoryMappedFile::~MemoryMappedFile()
{
    unmapFile();
}

MemoryMappedFile::MemoryMappedFile(const boost::filesystem::path &fileName)
 : _fileName(fileName),
   _fd(0),
   _mapping(nullptr),
   _size(0)
{}

bool MemoryMappedFile::mapFile(size_t minimumFileSize)
{
    bool didCreateFile = false;
    
    unmapFile();
    
    if ((_fd = open(_fileName.c_str(), O_RDWR | O_CREAT, 0)) < 0) {
        throw Exception("Failed to open file: %s with errno=%d", _fileName.c_str(), errno);
    }
    
    if (fchmod(_fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP) < 0) {
        throw Exception("Failed to chmod file: %s with errno=%d", _fileName.c_str(), errno);
    }
    
    off_t fileSize;
    {
        struct stat s;
        if (fstat(_fd, &s) < 0) {
            close(_fd);
            throw Exception("Failed to stat file: %s with errno=%d", _fileName.c_str(), errno);
        }
        fileSize = s.st_size;
    }
    
    if (fileSize == 0) {
        didCreateFile = true;
    }
    
    if (fileSize < minimumFileSize) {
        if (ftruncate(_fd, minimumFileSize) < 0) {
            close(_fd);
            throw Exception("Failed to resize file: %s with errno=%d", _fileName.c_str(), errno);
        }
        fileSize = minimumFileSize;
    }
    
    _size = fileSize;
    _mapping = (uint8_t *)mmap(nullptr,
                               fileSize,
                               PROT_READ | PROT_WRITE,
                               MAP_SHARED,
                               _fd,
                               0);
    
    if (MAP_FAILED == _mapping) {
        close(_fd);
        throw Exception("Failed to map file: %s with errno=%d", _fileName.c_str(), errno);
    }
    
    return didCreateFile;
}

void MemoryMappedFile::unmapFile()
{
    if (_mapping) {
        if (munmap(_mapping, _size) < 0) {
            SDL_Log("Failed to unmap file.");
        }
        
        _mapping = nullptr;
        _size = 0;
    }
    
    if (_fd) {
        if (close(_fd) < 0) {
            SDL_Log("Failed to close file.");
        }
        _fd = 0;
    }
}
