//
//  ManagedMallocZone.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/11/17.
//
//

#ifndef ManagedMallocZone_hpp
#define ManagedMallocZone_hpp

#include "BlockDataStore/BoxedMallocZone.hpp"
#include "MemoryMappedFile.hpp"
#include "Exception.hpp"
#include <boost/filesystem.hpp>


class ManagedMallocZoneException : public Exception
{
public:
    template<typename... Args>
    ManagedMallocZoneException(Args&&... args)
    : Exception(std::forward<Args>(args)...)
    {}
};

class ManagedMallocZoneMagicNumberException : public ManagedMallocZoneException
{
public:
    ManagedMallocZoneMagicNumberException(unsigned expected, unsigned actual)
    : ManagedMallocZoneException("Unexpected magic number in "
                                 "managed file: found {} but "
                                 "expected {}",
                                 expected, actual)
    {}
};

class ManagedMallocZoneVersionNumberException : public ManagedMallocZoneException
{
public:
    ManagedMallocZoneVersionNumberException(unsigned expected, unsigned actual)
    : ManagedMallocZoneException("Unexpected version number in "
                                 "managed file: found {} but "
                                 "expected {}",
                                 expected, actual)
    {}
};

class ManagedMallocZone
{
public:
    struct Header
    {
        uint32_t magic;
        uint32_t version;
        uint32_t lookupTableOffset;
        uint32_t zoneSize;
        uint8_t zoneData[0];
    };
    
    ManagedMallocZone(std::shared_ptr<spdlog::logger> log,
                      const boost::filesystem::path &regionFileName,
                      size_t initialFileSize,
                      uint32_t magic, uint32_t version);
    
    BoxedMallocZone::ConstBoxedBlock blockPointerForOffset(BoxedMallocZone::Offset offset) const;
    BoxedMallocZone::BoxedBlock blockPointerForOffset(BoxedMallocZone::Offset offset);
    BoxedMallocZone::BoxedBlock allocate(size_t size);
    void deallocate(BoxedMallocZone::BoxedBlock &&block);
    BoxedMallocZone::BoxedBlock reallocate(BoxedMallocZone::BoxedBlock &&block, size_t newSize);
    
    inline Header* header()
    {
        return (Header *)_file.mapping();
    }
    
    inline const Header* header() const
    {
        return (const Header *)_file.mapping();
    }
    
private:
    const uint32_t _magic;
    const uint32_t _version;
    
    MemoryMappedFile _file;
    BoxedMallocZone _zone;
    
    void mapFile(size_t minimumFileSize);
    void unmapFile();
    void growBackingMemory();
};

#endif /* ManagedMallocZone_hpp */
