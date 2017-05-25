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
    GridView(const GridAddressable<TYPE> &backing, const AABB &subregion)
     : _backing(backing),
       _subregion(subregion),
       _res(backing.getCellsInRegion(subregion))
    {}
    
    const TYPE& get(const glm::vec3 &p) const override
    {
        if (!this->inbounds(p)) {
            throw Exception("out of bounds");
        }
        return _backing.get(p);
    }
    
    glm::vec3 getCellDimensions() const override
    {
        return _backing.getCellDimensions();
    }
    
    AABB getBoundingBox() const override
    {
        return _subregion;
    }
    
    glm::ivec3 getResolution() const override
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
    GridViewMutable(GridMutable<TYPE> &backing, const AABB &subregion)
     : _backing(backing),
       _subregion(subregion),
       _res(backing.getCellsInRegion(subregion))
    {}
    
    const TYPE& get(const glm::vec3 &p) const override
    {
        if (!this->inbounds(p)) {
            throw Exception("out of bounds");
        }
        return _backing.get(p);
    }
    
    TYPE& mutableReference(const glm::vec3 &p) override
    {
        if (!this->inbounds(p)) {
            throw Exception("out of bounds");
        }
        return _backing.mutableReference(p);
    }
    
    void set(const glm::vec3 &p, const TYPE &object) override
    {
        if (!this->inbounds(p)) {
            throw Exception("out of bounds");
        }
        return _backing.set(p, object);
    }
    
    glm::vec3 getCellDimensions() const override
    {
        return _backing.getCellDimensions();
    }
    
    AABB getBoundingBox() const override
    {
        return _subregion;
    }
    
    glm::ivec3 getResolution() const override
    {
        return _res;
    }
    
private:
    GridMutable<TYPE> &_backing;
    const AABB _subregion;
    const glm::ivec3 _res;
};

#endif /* GridView_hpp */
