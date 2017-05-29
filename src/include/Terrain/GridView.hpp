//
//  GridView.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/23/17.
//
//

#ifndef GridView_hpp
#define GridView_hpp

#include "Terrain/GridAddressable.hpp"

// Constrains access to a single underlying GridAddressable object.
template<typename TYPE> class GridView : public GridAddressable<TYPE>
{
public:
    using GridAddressable<TYPE>::inbounds;
    
    GridView(const GridAddressable<TYPE> &backing, const AABB &subregion)
     : _backing(backing),
       _subregion(subregion),
       _res(backing.countCellsInRegion(subregion))
    {}
    
    const TYPE& get(const glm::vec3 &p) const override
    {
        if (!inbounds(p)) {
            throw OutOfBoundsException();
        }
        return _backing.get(p);
    }
    
    glm::vec3 cellDimensions() const override
    {
        return _backing.cellDimensions();
    }
    
    AABB boundingBox() const override
    {
        return _subregion;
    }
    
    glm::ivec3 gridResolution() const override
    {
        return _res;
    }
    
private:
    const GridAddressable<TYPE> &_backing;
    const AABB _subregion;
    const glm::ivec3 _res;
};

// Constrains access to a single underlying GridMutable object.
template<typename TYPE> class GridViewMutable : public GridMutable<TYPE>
{
public:
    using GridAddressable<TYPE>::inbounds;
    
    GridViewMutable(GridMutable<TYPE> &backing, const AABB &subregion)
     : _backing(backing),
       _subregion(subregion),
       _res(backing.countCellsInRegion(subregion))
    {}
    
    const TYPE& get(const glm::vec3 &p) const override
    {
        if (!inbounds(p)) {
            throw OutOfBoundsException();
        }
        return _backing.get(p);
    }
    
    TYPE& mutableReference(const glm::vec3 &p) override
    {
        if (!inbounds(p)) {
            throw OutOfBoundsException();
        }
        return _backing.mutableReference(p);
    }
    
    void set(const glm::vec3 &p, const TYPE &object) override
    {
        if (!inbounds(p)) {
            throw OutOfBoundsException();
        }
        return _backing.set(p, object);
    }
    
    glm::vec3 cellDimensions() const override
    {
        return _backing.cellDimensions();
    }
    
    AABB boundingBox() const override
    {
        return _subregion;
    }
    
    glm::ivec3 gridResolution() const override
    {
        return _res;
    }
    
private:
    GridMutable<TYPE> &_backing;
    const AABB _subregion;
    const glm::ivec3 _res;
};

#endif /* GridView_hpp */
