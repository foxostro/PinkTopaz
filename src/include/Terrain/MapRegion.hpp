//
//  MapRegion.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/7/17.
//
//

#ifndef MapRegion_hpp
#define MapRegion_hpp

#include "Terrain/Voxel.hpp"
#include "Terrain/VoxelDataSerializer.hpp"
#include "Grid/Array3D.hpp"
#include "BlockDataStore/BlockDataStore.hpp"
#include <spdlog/spdlog.h>

// Stores/Loads voxel chunks on the file system.
class MapRegion
{
public:
    MapRegion(std::shared_ptr<spdlog::logger> log,
              const boost::filesystem::path &regionFileName);
    
    // Loads a voxel chunk from file, if available.
    // The key uniquely identifies the chunk in the voxel chunk in space.
    boost::optional<Array3D<Voxel>> load(const AABB &bbox, Morton3 key);
    
    // Stores a voxel chunk to file.
    // The key uniquely identifies the chunk in the voxel chunk in space.
    void store(Morton3 key, const Array3D<Voxel> &voxels);
    
private:
    VoxelDataSerializer _serializer;
    BlockDataStore _dataStore;
    std::shared_ptr<spdlog::logger> _log;
};

#endif /* MapRegion_hpp */
