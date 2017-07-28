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

void TerrainMeshGrid::readerTransactionTry(const AABB &region, const std::function<void(const TerrainMesh &terrainMesh)> &onPresent) const
{
    assert(_array);
    
    // For items which are present, we store the cell, a reference to the
    // terrain mesh, and a reference to the lock.
    class Item
    {
    private:
        // Set to true when we've taken the lock. Initially false.
        bool _ownsLock;
        
        // Reference to the lock itself.
        std::reference_wrapper<std::mutex> _lock;
        
        // This is a reference to the terrain mesh for the item.
        // This is an optional so that we can leave it empty until after
        // we've tried to take the lock. If we've taken the lock then we
        // expect this to be valid.
        std::experimental::optional<std::reference_wrapper<const TerrainMesh>> _ref;
        
    public:
        Item(std::mutex &m)
         : _ownsLock(false), _lock(m)
        {}
        
        // If we move the item into a LockVector then we need to mark the
        // lock as not being owned by the item anymore. This way, the lock
        // gets released one time when the LockVector is destroyed.
        Item(Item &&item)
         : _ownsLock(item._ownsLock),
           _lock(item._lock),
           _ref(item._ref)
        {
            item._ownsLock = false;
            item._ref = std::experimental::nullopt;
        }
        
        ~Item()
        {
            unlock();
        }
        
        inline void unlock()
        {
            if (_ownsLock) {
                std::mutex &m = _lock;
                m.unlock();
                _ownsLock = false;
            }
        }
        
        inline bool try_lock()
        {
            std::mutex &m = _lock;
            _ownsLock = m.try_lock();
            return _ownsLock;
        }
        
        inline const TerrainMesh &getTerrainMesh() const
        {
            assert(_ref);
            return *_ref;
        }
        
        inline void setTerrainMesh(const TerrainMesh &terrainMesh)
        {
            _ref = std::experimental::optional<std::reference_wrapper<const TerrainMesh>>(terrainMesh);
        }
    };
    
    class ItemVector
    {
    public:
        std::vector<Item> items;
        
        ~ItemVector()
        {
            // To avoid deadlock we must release locks in the reverse order
            // in which we took them.
            for (auto iter = items.rbegin(); iter != items.rend(); ++iter) {
                iter->unlock();
            }
        }
    };
    
    {
        // Locks are released in the correct order when `presentItems' goes out
        // of scope and is destroyed.
        ItemVector presentItems;
        
        // For all cells in the specified region, get the present items and
        // stash them in `present'. Items for which we can get the lock, and for
        // which there is a terrain mesh actually present, are recognized as
        // being present. To avoid deadlock, we assume that mutableForEachCell()
        // will hand us the associated locks in the canonical order.
        _arrayLocks.mutableForEachCell(region, [&](const AABB &cell,
                                                   Morton3 index,
                                                   std::mutex &lock){
            Item item(lock);
            
            if (item.try_lock()) {
                MaybeTerrainMesh &maybe = _array->mutableReference(cell.center);
                if (maybe) {
                    item.setTerrainMesh(*maybe);
                    
                    // By moving the item, we ensure lock ownership follows the
                    // item into the ItemVector. In the cases where we never
                    // don't do this move, the locks are released as soon as
                    // `item' goes out of scope and is destroyed.
                    presentItems.items.emplace_back(std::move(item));
                }
            }
        });
        
        // For all items which are present, execute the onPresent callable.
        for (const auto &item : presentItems.items) {
            onPresent(item.getTerrainMesh());
        }
    }
}
