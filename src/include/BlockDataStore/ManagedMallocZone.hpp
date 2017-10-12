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
#include <boost/filesystem.hpp>

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
    
    ManagedMallocZone(const boost::filesystem::path &regionFileName,
                      size_t initialFileSize);
    
    BoxedMallocZone::BoxedBlock blockPointerForOffset(BoxedMallocZone::Offset offset);
    BoxedMallocZone::BoxedBlock allocate(size_t size);
    void deallocate(BoxedMallocZone::BoxedBlock &&block);
    BoxedMallocZone::BoxedBlock reallocate(BoxedMallocZone::BoxedBlock &&block, size_t newSize);
    
    inline Header* header()
    {
        return (Header *)_file.mapping();
    }
    
private:
    static constexpr uint32_t MANAGED_MALLOC_ZONE_MAGIC = 'rpam';
    static constexpr uint32_t MANAGED_MALLOC_ZONE_VERSION = 0;
    
    MemoryMappedFile _file;
    BoxedMallocZone _zone;
    
    void mapFile(size_t minimumFileSize);
    void unmapFile();
    void growBackingMemory();
};

#endif /* ManagedMallocZone_hpp */
