//
//  GridAddressable.hpp
//  PinkTopaz
//
//  Created by Andrew Fox on 5/20/17.
//
//

#ifndef GridAddressable_hpp
#define GridAddressable_hpp

#include <glm/vec3.hpp>

// A regular grid in space where each cell is associated with some object of the templated type.
template<typename TYPE> class GridAddressable
{
public:
    virtual ~GridAddressable() = default;
    
    // Each point in space corresponds to exactly one cell. Get the object.
    virtual const TYPE& get(const glm::vec3 &p) const = 0;
    
    // Each point in space corresponds to exactly one cell. Get the object.
    // If the point is not in bounds then return the specified default value.
    virtual const TYPE& get(const glm::vec3 &p, const TYPE &defaultValue) const = 0;
    
    // Each point in space corresponds to exactly one cell. Get the (mutable) object.
    virtual TYPE& getm(const glm::vec3 &p) = 0;
    
    // Each point in space corresponds to exactly one cell. Set the object.
    virtual void set(const glm::vec3 &p, const TYPE &object) = 0;
    
    // Gets the dimensions of a single cell. (All cells are the same size.)
    virtual glm::vec3 getCellDimensions() const = 0;
    
    // Gets the region for which the grid is defined.
    virtual AABB getBoundingBox() const = 0;
    
    // Gets the number of cells along each axis within the valid region.
    virtual glm::ivec3 getResolution() const = 0;
};

#endif /* GridAddressable_hpp */
