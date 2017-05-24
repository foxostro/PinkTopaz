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
        assert(this->inbounds(p));
        return _backing.get(p);
    }
    
    const TYPE& get(const glm::vec3 &p, const TYPE &defaultValue) const override
    {
        if (!this->inbounds(p)) {
            return defaultValue;
        } else {
            return _backing.get(p, defaultValue);
        }
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
        assert(this->inbounds(p));
        return _backing.get(p);
    }
    
    const TYPE& get(const glm::vec3 &p, const TYPE &defaultValue) const override
    {
        if (!this->inbounds(p)) {
            return defaultValue;
        } else {
            return _backing.get(p, defaultValue);
        }
    }
    
    TYPE& mutableReference(const glm::vec3 &p) override
    {
        assert(this->inbounds(p));
        return _backing.mutableReference(p);
    }
    
    void set(const glm::vec3 &p, const TYPE &object) override
    {
        assert(this->inbounds(p));
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
