//
//  VoxelDataStore.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/16/17.
//
//

#include "Terrain/VoxelDataStore.hpp"
#include "Profiler.hpp"

VoxelDataStore::VoxelDataStore()
 : _data(_generator),
   _chunkLocks(_generator.boundingBox(), _generator.gridResolution())
{}

VoxelDataStore::Locks VoxelDataStore::locksForRegion(const AABB &region) const
{
    std::lock_guard<std::mutex> lock(_lockChunkLocks);
    Locks locks;
    _chunkLocks.mutableForEachCell(region, [&](const AABB &cell,
                                               Morton3 index,
                                               std::shared_ptr<std::mutex> &cellLock){
        if (!cellLock) {
            cellLock = std::make_shared<std::mutex>();
        }
        locks.push_back(cellLock);
    });
    
    return locks;
}

void VoxelDataStore::acquireLocks(const Locks &locks) const
{
    for (auto &lock : locks) {
        lock->lock();
    }
}

void VoxelDataStore::releaseLocks(const Locks &locks) const
{
    for (auto iter = locks.rbegin(); iter != locks.rend(); ++iter) {
        auto &cellLock = *iter;
        cellLock->unlock();
    }
}

void VoxelDataStore::readerTransaction(const AABB &region, const Reader &fn) const
{
    // AFOX_TODO: This isn't exception-safe. I'm not sure if I care.
    PROFILER(VoxelDataStoreReader);
    auto locks = locksForRegion(region);
    acquireLocks(locks);
    fn(_data.copy(region));
    releaseLocks(locks);
}

void VoxelDataStore::writerTransaction(const AABB &region, const Writer &fn)
{
    // AFOX_TODO: This isn't exception-safe. I'm not sure if I care.
    PROFILER(VoxelDataStoreWriter);
    auto locks = locksForRegion(region);
    acquireLocks(locks);
    ChangeLog changeLog = fn(_data);
    releaseLocks(locks);
    voxelDataChanged(changeLog);
}

glm::vec3 VoxelDataStore::cellDimensions() const
{
    return _data.cellDimensions();
}

AABB VoxelDataStore::boundingBox() const
{
    static constexpr size_t TERRAIN_CHUNK_SIZE = 16; // AFOX_TODO: Use one TERRAIN_CHUNK_SIZE for entire app.
    const glm::vec3 chunkSize(TERRAIN_CHUNK_SIZE, TERRAIN_CHUNK_SIZE, TERRAIN_CHUNK_SIZE);
    AABB box = _generator.boundingBox().inset(glm::vec3(16.f, 16.f, 16.f));
    return box;
}

glm::ivec3 VoxelDataStore::gridResolution() const
{
    const size_t TERRAIN_CHUNK_SIZE = 16; // AFOX_TODO: Use one TERRAIN_CHUNK_SIZE for entire app.
    const glm::ivec3 chunkSize(TERRAIN_CHUNK_SIZE, TERRAIN_CHUNK_SIZE, TERRAIN_CHUNK_SIZE);
    glm::ivec3 res = _generator.gridResolution() - chunkSize*2;
    return res;
}
