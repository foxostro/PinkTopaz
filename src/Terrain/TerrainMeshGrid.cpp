//
//  TerrainMeshGrid.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 7/27/17.
//
//

#include "Terrain/TerrainMeshGrid.hpp"

TerrainMeshGrid::TerrainMeshGrid(std::unique_ptr<GridMutable<MaybeTerrainMesh>> &&array,
                                 unsigned lockGridResDivisor)
 : ConcurrentGridMutable<MaybeTerrainMesh>(std::move(array), lockGridResDivisor)
{}

void TerrainMeshGrid::readerTransactionTry(const AABB &region,
                                           const std::function<void(const TerrainMesh &terrainMesh)> &onPresent,
                                           const std::function<void(const AABB &cell)> &onMissing) const
{
    assert(_array);
    
    // For items which are missing we store only the cell.
    std::vector<AABB> missingItems;
    
    {
        // For items which are present, we store the cell, a reference to the
        // terrain mesh, and a reference to the lock.
        class Item
        {
        private:
            // Set to true when we've taken the lock. Initially false.
            bool ownsLock;
            
            // Reference to the lock itself.
            std::reference_wrapper<std::mutex> lock;
            
            // This is a reference to the terrain mesh for the item.
            // This is an optional so that we can leave it empty until after
            // we've tried to take the lock. If we've taken the lock then we
            // expect this to be valid.
            std::experimental::optional<std::reference_wrapper<const TerrainMesh>> maybeTerrainMeshRef;
            
        public:
            Item(std::mutex &m)
             : ownsLock(false), lock(m)
            {}
            
            ~Item()
            {
                if (ownsLock) {
                    std::mutex &m = lock;
                    m.unlock();
                }
            }
            
            bool try_lock()
            {
                std::mutex &m = lock;
                ownsLock = m.try_lock();
                return ownsLock;
            }
            
            const TerrainMesh &getTerrainMesh() const
            {
                assert(maybeTerrainMeshRef);
                return *maybeTerrainMeshRef;
            }
            
            void setTerrainMesh(const TerrainMesh &terrainMesh)
            {
                maybeTerrainMeshRef = std::experimental::optional<std::reference_wrapper<const TerrainMesh>>(terrainMesh);
            }
        };
        std::vector<Item> presentItems;
        
        // For all cells in the specified region, get the present and missing
        // items and stash them in `present' and `missing', respetively. Items
        // for which we can get the lock, and for which there is a terrain mesh
        // actually present, are recognized as being present. All other items
        // are recognized as being missing. To avoid deadlock, we assume that
        // mutableForEachCell() will hand us the associated locks in the
        // canonical order.
        _arrayLocks.mutableForEachCell(region, [&](const AABB &cell,
                                                   Morton3 index,
                                                   std::mutex &lock){
            bool thisItemIsMissing = true;
            
            Item item(lock);
            
            if (item.try_lock()) {
                MaybeTerrainMesh &maybe = _array->mutableReference(cell.center);
                if (maybe) {
                    item.setTerrainMesh(*maybe);
                    presentItems.emplace_back(item);
                    thisItemIsMissing = false;
                }
            }
            
            if (thisItemIsMissing) {
                missingItems.emplace_back(cell);
            }
        });
        
        // For all items which are present, execute the onPresent callable.
        for (const auto &item : presentItems) {
            onPresent(item.getTerrainMesh());
        }
        
        // Locks are released when `presentItems' goes out of scope and is
        // destroyed.
    }
    
    // For all missing items, execute the onMissing callable.
    for (const AABB &cell : missingItems) {
        onMissing(cell);
    }
}
