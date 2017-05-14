#include <iostream>

#include "gtest/gtest.h"
#include "Terrain/VoxelData.hpp"

TEST(Zero, VoxelDataIndex)
{
    const AABB box = {
        glm::vec3(16.f, 16.f, 16.f),
        glm::vec3(16.f, 16.f, 16.f),
    };
    const glm::ivec3 res(32, 32, 32);
    Terrain::VoxelData data(box, res);
    
    EXPECT_EQ(0, data.indexAtPoint(glm::vec3(0.0f, 0.0f, 0.0f)));
}

TEST(IterateInOrder, VoxelDataIndex)
{
    // The region of space for which voxel data is defined.
    const AABB box = {
        glm::vec3(10.f, 10.f, 10.f),
        glm::vec3(10.f, 10.f, 10.f),
    };
    const glm::vec3 mins = box.center - box.extent;
    const glm::vec3 maxs = box.center + box.extent;
    
    // Rsolution of the voxel grid.
    const glm::ivec3 res(10, 10, 10);
    
    // Increments between voxel cells.
    const glm::vec3 step(box.extent.x*2.0f / res.x,
                         box.extent.y*2.0f / res.y,
                         box.extent.z*2.0f / res.z);
    
    Terrain::VoxelData data(box, res);
    
    // Iterate over all cells, in order.
    size_t index = 0;
    glm::vec3 p;
    for(p.x = mins.x; p.x < maxs.x; p.x += step.x)
    {
        for(p.z = mins.z; p.z < maxs.z; p.z += step.z)
        {
            for(p.y = mins.y; p.y < maxs.y; p.y += step.y)
            {
                size_t c = data.indexAtPoint(p);
                EXPECT_EQ(index, c);
                index++;
            }
        }
    }
}
