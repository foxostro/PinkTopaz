//
//  MallocTests.cpp
//  PinkTopaz
//
//  Created by Andrew Fox on 10/4/17.
//
//

#include "catch.hpp"
#include "BlockDataStore/MallocZone.hpp"

#include <cstdlib>
#include <cstdio>
#include <cstdbool>
#include <cstddef>
#include <cassert>
#include <cstring>

constexpr size_t SMALL = 64;

static uint8_t g_buffer[1024];

TEST_CASE("Test Init", "[Malloc]") {
    const size_t size = sizeof(g_buffer);

    memset(g_buffer, 0, size);
    MallocZone zone;
    zone.reset(g_buffer, size);

    // zone initially contains one large empty block
    MallocZone::Block *head = zone.head();
    REQUIRE(head != nullptr);
    REQUIRE(zone.prev(head) == nullptr);
    REQUIRE(zone.next(head) == nullptr);
    REQUIRE(head->size < size);
    REQUIRE(!head->inuse);
}

// Starting with an empty zone, we cannot satisfy an allocation request larger
// than the size of the zone itself.
TEST_CASE("Test Malloc Really Big", "[Malloc]") {
    memset(g_buffer, 0, sizeof(g_buffer));
    MallocZone zone;
    zone.reset(g_buffer, sizeof(g_buffer));
    MallocZone::Block *allocation = zone.allocate(SIZE_MAX);
    REQUIRE(allocation == nullptr);
}

// Starting with an empty zone, we should be able to satisfy a small request.
TEST_CASE("Test Malloc One Small Request", "[Malloc]") {
    memset(g_buffer, 0, sizeof(g_buffer));
    MallocZone zone;
    zone.reset(g_buffer, sizeof(g_buffer));
    MallocZone::Block *allocation = zone.allocate(SMALL);
    REQUIRE(allocation != nullptr);
    REQUIRE(allocation->size >= SMALL);
    REQUIRE((uintptr_t)allocation % 4 == 0);
}

// Starting with an empty zone, we should be able to satisfy a request for a
// size zero block. This returns a minimum size heap block.
TEST_CASE("Test Malloc One Zero Size Request", "[Malloc]") {
    memset(g_buffer, 0, sizeof(g_buffer));
    MallocZone zone;
    zone.reset(g_buffer, sizeof(g_buffer));
    MallocZone::Block *allocation = zone.allocate(0);
    REQUIRE(allocation != nullptr);
    REQUIRE(allocation->size >= 0);
    REQUIRE((uintptr_t)allocation % 4 == 0);
}

// Starting with an empty zone, we should be able to satisfy a request the size
// of the zone's free space.
TEST_CASE("Test Malloc Whole Thing", "[Malloc]") {
    memset(g_buffer, 0, sizeof(g_buffer));
    MallocZone zone;
    zone.reset(g_buffer, sizeof(g_buffer));
    size_t size = zone.head()->size;
    MallocZone::Block *allocation = zone.allocate(size);
    REQUIRE(allocation != nullptr);
    REQUIRE(allocation->size == size);
    REQUIRE((uintptr_t)allocation % 4 == 0);
}

// Starting with an empty zone, we should be able to several small requests.
TEST_CASE("Test Malloc Several Small", "[Malloc]") {
    memset(g_buffer, 0, sizeof(g_buffer));
    MallocZone zone;
    zone.reset(g_buffer, sizeof(g_buffer));

    size_t size = SMALL, i = 0;
    size_t ex = (sizeof(g_buffer) - sizeof(MallocZone::Block)) / (sizeof(MallocZone::Block) + size);

    while (true) {
        MallocZone::Block *allocation = zone.allocate(size);
        REQUIRE((uintptr_t)allocation % 4 == 0);
        if (!allocation) {
            break;
        }
        ++i;
    }

    REQUIRE(i >= ex);
}

// We should be able to allocate one, free one, and then allocate another.
TEST_CASE("Test Malloc One Free one", "[Malloc]") {
    memset(g_buffer, 0, sizeof(g_buffer));
    MallocZone zone;
    zone.reset(g_buffer, sizeof(g_buffer));
    MallocZone::Block *alloc = zone.allocate(zone.head()->size);
    REQUIRE(alloc != nullptr);
    REQUIRE(zone.allocate(zone.head()->size) == nullptr);
    zone.deallocate(alloc);
    alloc = zone.allocate(zone.head()->size);
    REQUIRE(alloc != nullptr);
}

// Freeing allocations should release memory to the zone for future allocations.
TEST_CASE("Test Malloc Several Free One", "[Malloc]") {
    memset(g_buffer, 0, sizeof(g_buffer));
    MallocZone zone;
    zone.reset(g_buffer, sizeof(g_buffer));
    MallocZone::Block *allocation = zone.allocate(SMALL);
    while (zone.allocate(SMALL));
    zone.deallocate(allocation);
    allocation = zone.allocate(SMALL);
    REQUIRE(allocation != nullptr);
}

// Free blocks should coalesce to avoid fragmentation.
// Allocate three blocks to fill the zone. Free two and then see whether we can
// use the coalesced free space to allocate a new block. This tests merging of
// a preceding free block.
TEST_CASE("Test Coalesce 0", "[Malloc]") {
    memset(g_buffer, 0, sizeof(g_buffer));
    MallocZone zone;
    zone.reset(g_buffer, sizeof(g_buffer));

    MallocZone::Block *a = zone.allocate(SMALL);
    REQUIRE(a);

    MallocZone::Block *b = zone.allocate(SMALL);
    REQUIRE(b);

    const size_t remainingSpace = sizeof(g_buffer) -
                                  sizeof(MallocZone::Header) - 
                                  3*sizeof(MallocZone::Block) - 
                                  2*SMALL;
    MallocZone::Block *c = zone.allocate(remainingSpace);
    REQUIRE(c);

    MallocZone::Block *d = zone.allocate(SMALL*2);
    REQUIRE(!d); // expected to fail

    zone.deallocate(a);
    zone.deallocate(b); // merge with preceding free block

    MallocZone::Block *e = zone.allocate(SMALL*2);
    REQUIRE(e); // should succeed now
    REQUIRE(a == e); // using the same block as `a'
}

// Free blocks should coalesce to avoid fragmentation.
// Allocate three blocks to fill the zone. Free two and then see whether we can
// use the coalesced free space to allocate a new block. This tests merging of
// a following free block.
TEST_CASE("Test Coalesce 1", "[Malloc]") {
    memset(g_buffer, 0, sizeof(g_buffer));
    MallocZone zone;
    zone.reset(g_buffer, sizeof(g_buffer));

    MallocZone::Block *a = zone.allocate(SMALL);
    REQUIRE(a);

    MallocZone::Block *b = zone.allocate(SMALL);
    REQUIRE(b);

    const size_t remainingSpace = sizeof(g_buffer) -
                                  sizeof(MallocZone::Header) - 
                                  2*sizeof(MallocZone::Block) -
                                  2*SMALL;
    MallocZone::Block *c = zone.allocate(remainingSpace);
    REQUIRE(c);

    MallocZone::Block *d = zone.allocate(SMALL*2);
    REQUIRE(!d); // expected to fail

    zone.deallocate(b);
    zone.deallocate(a); // merge with the following free block

    MallocZone::Block *e = zone.allocate(SMALL*2);
    REQUIRE(e); // should succeed now
    REQUIRE(a == e); // using the same block as `a'
}

// Free blocks should coalesce to avoid fragmentation.
// Allocate three blocks to fill the zone. Free three and then see whether we
// can use the coalesced free space to allocate a new block. This tests merging
// a preceding and following free block in one step.
TEST_CASE("Test Coalesce 2", "[Malloc]") {
    memset(g_buffer, 0, sizeof(g_buffer));
    MallocZone zone;
    zone.reset(g_buffer, sizeof(g_buffer));

    MallocZone::Block *a = zone.allocate(SMALL);
    REQUIRE(a);

    MallocZone::Block *b = zone.allocate(SMALL);
    REQUIRE(b);

    const size_t remainingSpace = sizeof(g_buffer) -
                                  sizeof(MallocZone::Header) - 
                                  3*sizeof(MallocZone::Block) -
                                  2*SMALL;
    MallocZone::Block *c = zone.allocate(remainingSpace);
    REQUIRE(c);

    MallocZone::Block *d = zone.allocate(sizeof(g_buffer) - sizeof(MallocZone::Header) - sizeof(MallocZone::Block));
    REQUIRE(!d); // expected to fail

    zone.deallocate(c);
    zone.deallocate(a);
    zone.deallocate(b); // preceding and following both merged here

    MallocZone::Block *e = zone.allocate(sizeof(g_buffer) - sizeof(MallocZone::Header) - sizeof(MallocZone::Block));
    REQUIRE(e); // should succeed now
    REQUIRE(a == e); // using the same block as `a'
}

// Realloc extends a live allocation into the free space following it.
// If the block already has capacity then there's no need to change anything.
TEST_CASE("Test Realloc Extend 0", "[Malloc]") {
    memset(g_buffer, 0, sizeof(g_buffer));
    MallocZone zone;
    zone.reset(g_buffer, sizeof(g_buffer));

    MallocZone::Block *a = zone.allocate(1);
    REQUIRE(a);
    REQUIRE(zone.next(zone.head()) != nullptr);

    MallocZone::Block *block1 = zone.head();
    MallocZone::Block *block2 = zone.next(zone.head());

    size_t size1 = block1->size;
    size_t size2 = block2->size;

    MallocZone::Block *b = zone.reallocate(a, 2);
    REQUIRE(b);
    REQUIRE(a == b);

    REQUIRE(zone.head() == block1);
    REQUIRE(zone.next(zone.head()) == block2);
    REQUIRE(block1->size == size1);
    REQUIRE(block2->size == size2);
}

// If realloc cannot extend the current allocation into the following block
// then it allocates another chunk of memory for the new allocation and moves
// it there.
TEST_CASE("Test Realloc Extend 1", "[Malloc]") {
    memset(g_buffer, 0, sizeof(g_buffer));
    MallocZone zone;
    zone.reset(g_buffer, sizeof(g_buffer));

    MallocZone::Block *a = zone.allocate(0);
    REQUIRE(a);
    REQUIRE(zone.next(zone.head()) != nullptr);

    MallocZone::Block *b = zone.reallocate(a, SMALL);
    REQUIRE(b);
    REQUIRE(a == b);

    MallocZone::Block *block1 = zone.head();
    REQUIRE(block1);

    MallocZone::Block *block2 = zone.next(zone.head());
    REQUIRE(block2);

    REQUIRE(block1->size >= SMALL);
    REQUIRE(block2->size >= sizeof(g_buffer) - sizeof(MallocZone::Header) - 2*sizeof(MallocZone::Block) - SMALL);
}

// Realloc tries to extend an allocation in place. If it cannot then it
// allocates a new block and moves the allocation.
TEST_CASE("Test Realloc Relocate 0", "[Malloc]") {
    memset(g_buffer, 0, sizeof(g_buffer));
    MallocZone zone;
    zone.reset(g_buffer, sizeof(g_buffer));

    MallocZone::Block *a = zone.allocate(SMALL);
    REQUIRE(a);

    MallocZone::Block *b = zone.allocate(SMALL);
    REQUIRE(b);

    MallocZone::Block *c = zone.reallocate(a, 2*SMALL);
    REQUIRE(c);
    REQUIRE(a != c);

    size_t count = 0;
    for (MallocZone::Block *block = zone.head(); block; block = zone.next(block)) {
        ++count;
    }
    REQUIRE(count == 4);

    size_t expectedSize[] = {
        SMALL,
        SMALL,
        2*SMALL,
        sizeof(g_buffer) - sizeof(MallocZone::Header) - 4*SMALL - 3*sizeof(MallocZone::Block)
    };
    size_t expectedInUse[] = {
        false,
        true,
        true,
        false
    };
    count = 0;
    for (MallocZone::Block *block = zone.head(); block; block = zone.next(block)) {
        REQUIRE(block->size == expectedSize[count]);
        REQUIRE(block->inuse == expectedInUse[count]);
        ++count;
    }
}

// Realloc tries to extend an allocation in place. If it cannot then it
// allocates a new block and moves the allocation. However, if the second
// allocation fails then realloc returns nullptr.
TEST_CASE("Test Realloc Relocate 1", "[Malloc]") {
    memset(g_buffer, 0, sizeof(g_buffer));
    MallocZone zone;
    zone.reset(g_buffer, sizeof(g_buffer));

    MallocZone::Block *a = zone.allocate(SMALL);
    REQUIRE(a);

    MallocZone::Block *b = zone.allocate(SMALL);
    REQUIRE(b);

    const size_t remainingSpace = sizeof(g_buffer) -
                                  sizeof(MallocZone::Header) - 
                                  2*sizeof(MallocZone::Block) -
                                  2*SMALL;
    MallocZone::Block *c = zone.allocate(remainingSpace);
    REQUIRE(c);

    MallocZone::Block *c2 = zone.allocate(0); // should fail as heap is full
    REQUIRE(!c2);

    // The reallocation will fail because there isn't enough free space.
    // Block `a' remains valid in this case.
    MallocZone::Block *d = zone.reallocate(a, 2*SMALL);
    REQUIRE(!d);
    REQUIRE(a->inuse);

    size_t count = 0;
    for (MallocZone::Block *block = zone.head(); block; block = zone.next(block)) {
        ++count;
    }
    REQUIRE(count == 3);

    size_t expectedSize[] = {
        SMALL,
        SMALL,
        sizeof(g_buffer) - sizeof(MallocZone::Header) - 2*(SMALL+sizeof(MallocZone::Block))
    };
    size_t expectedInUse[] = {
        true,
        true,
        true
    };
    count = 0;
    for (MallocZone::Block *block = zone.head(); block; block = zone.next(block)) {
        REQUIRE(block->size == expectedSize[count]);
        REQUIRE(block->inuse == expectedInUse[count]);
        ++count;
    }
}

// Realloc tries to extend an allocation in place. If it cannot then it
// allocates a new block and moves the allocation. In the case where the
// preceding block has space then we merge the block into the preceding block
// and move the allocation. This allows realloc to succeed in the case where
// the only space large enough is the combined space of the preceding block
// and the current block.
TEST_CASE("Test Realloc Relocate 2", "[Malloc]") {
    memset(g_buffer, 0, sizeof(g_buffer));
    MallocZone zone;
    zone.reset(g_buffer, sizeof(g_buffer));

    MallocZone::Block *a = zone.allocate(SMALL);
    REQUIRE(a);

    MallocZone::Block *b = zone.allocate(SMALL);
    REQUIRE(b);

    const size_t remainingSpace = sizeof(g_buffer) -
                                  sizeof(MallocZone::Header) - 
                                  3*sizeof(MallocZone::Block) -
                                  2*SMALL;
    MallocZone::Block *c = zone.allocate(remainingSpace);
    REQUIRE(c);

    zone.deallocate(a);

    MallocZone::Block *d = zone.reallocate(b, 2*SMALL);
    REQUIRE(d);
    REQUIRE(a == d);

    size_t count = 0;
    for (MallocZone::Block *block = zone.head(); block; block = zone.next(block)) {
        ++count;
    }
    REQUIRE(count == 2);

    size_t expectedSize[] = {
        2*SMALL + sizeof(MallocZone::Block),
        sizeof(g_buffer) - sizeof(MallocZone::Header) - 2*sizeof(MallocZone::Block) - 2*SMALL
    };
    size_t expectedInUse[] = {
        true,
        true,
    };
    count = 0;
    for (MallocZone::Block *block = zone.head(); block; block = zone.next(block)) {
        REQUIRE(block->size == expectedSize[count]);
        REQUIRE(block->inuse == expectedInUse[count]);
        ++count;
    }
}

// Like "Test Realloc Relocate 2" but we're specifically testing the case where
// we extend the tail block backwards into the free space preceding it.
TEST_CASE("Test Realloc Relocate 3", "[Malloc]") {
    memset(g_buffer, 0, sizeof(g_buffer));
    MallocZone zone;
    zone.reset(g_buffer, sizeof(g_buffer));
    
    MallocZone::Block *a = zone.allocate(SMALL);
    REQUIRE(a);
    
    MallocZone::Block *b = zone.allocate(SMALL);
    REQUIRE(b);
    
    const size_t remainingSpace = sizeof(g_buffer) -
                                  sizeof(MallocZone::Header) -
                                  2*sizeof(MallocZone::Block) -
                                  2*SMALL;
    MallocZone::Block *c = zone.allocate(remainingSpace);
    REQUIRE(c);
    
    // Heap should be full now.
    REQUIRE(!zone.allocate(0));
    
    // Free up a hole directly behind the tail block.
    zone.deallocate(b);
    
    // If we merge the tail block into the free space preceding it then there
    // is just enough space for this allocation to succeed.
    MallocZone::Block *d = zone.reallocate(c, remainingSpace + SMALL + sizeof(MallocZone::Block));
    REQUIRE(d);
    REQUIRE(b == d);
    
    // TODO: Should I check that the contents were actually moved correctly?
    
    size_t count = 0;
    for (MallocZone::Block *block = zone.head(); block; block = zone.next(block)) {
        ++count;
    }
    REQUIRE(count == 2);
    
    size_t expectedSize[] = {
        SMALL,
        sizeof(g_buffer) - sizeof(MallocZone::Header) - sizeof(MallocZone::Block) - SMALL
    };
    size_t expectedInUse[] = {
        true,
        true,
    };
    count = 0;
    for (MallocZone::Block *block = zone.head(); block; block = zone.next(block)) {
        REQUIRE(block->size == expectedSize[count]);
        REQUIRE(block->inuse == expectedInUse[count]);
        ++count;
    }
}

// Realloc tries to extend an allocation in place. If it cannot then it
// allocates a new block and moves the allocation. In the case where the
// preceding block has space then we merge the block into the preceding block
// and move the allocation. This allows realloc to succeed in the case where
// the only space large enough is the combined space of the preceding block
// and the current block.
TEST_CASE("Test Realloc Shrink", "[Malloc]") {
    memset(g_buffer, 0, sizeof(g_buffer));
    MallocZone zone;
    zone.reset(g_buffer, sizeof(g_buffer));

    MallocZone::Block *a = zone.allocate(sizeof(g_buffer) - sizeof(MallocZone::Header) - sizeof(MallocZone::Block));
    REQUIRE(a);

    MallocZone::Block *d = zone.reallocate(a, SMALL);
    REQUIRE(d);
    REQUIRE(a == d);

    size_t count = 0;
    for (MallocZone::Block *block = zone.head(); block; block = zone.next(block)) {
        ++count;
    }
    REQUIRE(count == 2);

    size_t expectedSize[] = {
        SMALL,
        sizeof(g_buffer) - sizeof(MallocZone::Header) - sizeof(MallocZone::Block) - SMALL
    };
    size_t expectedInUse[] = {
        true,
        false,
    };
    count = 0;
    for (MallocZone::Block *block = zone.head(); block; block = zone.next(block)) {
        REQUIRE(block->size == expectedSize[count]);
        REQUIRE(block->inuse == expectedInUse[count]);
        ++count;
    }
}
