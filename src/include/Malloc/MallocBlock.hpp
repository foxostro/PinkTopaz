//
//  MallocBlock.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/4/17.
//
//

#ifndef MallocBlock_hpp
#define MallocBlock_hpp

#include <cstddef>
#include <cstdbool>
#include <cstdint>

struct MallocBlock {
    MallocBlock *prev;
    MallocBlock *next;
    size_t size;
    unsigned inuse;
};

#endif /* MallocBlock_hpp */
