// Project
#include <Pool.h>
#include <Block.h>

// STL
#include <iostream>
#include <cassert>

static void EXPECT(const char* message, bool testCondition)
{
    testCondition
        ? std::cout << "[Passed] Expect | " << message << std::endl
        : std::cout << "[Failed] Expect | " << message << "\n\n" << std::endl;
    assert(testCondition);
}

int main()
{
    constexpr size_t BLOCK_COUNT = 1;
    constexpr size_t BLOCK_SIZE = 16;
    constexpr size_t TOTAL_BYTES_IN_POOL = BLOCK_COUNT * BLOCK_SIZE;
    const char* TEST_STRING_1 = "Test String";
    const char* TEST_STRING_2 = "Example Here";
    const char* SUPER_LONG_STRING = "Super Long Test PoolString is long";

    Pool<Block<BLOCK_SIZE>, BLOCK_COUNT> pool;

    // Verify the size of the is pool correct
    const bool hasCorrectSize = pool.numBytes() == TOTAL_BYTES_IN_POOL;
    EXPECT("Pool size to be equal to the number of blocks multiplied by the pool block size", hasCorrectSize);


    // Allocate multiple identical strings
    // Should reuse the same block
    [&]() -> void {
        PoolString firstInstance = pool.findOrCreate(TEST_STRING_1);
        EXPECT("First instance of PoolString to have 1 occurrence", pool.countFor(TEST_STRING_1) == 1);
        PoolString secondInstance = pool.findOrCreate(TEST_STRING_1);
        EXPECT("Second instance of PoolString to have 2 occurrence", pool.countFor(TEST_STRING_1) == 2);
    }();
    EXPECT("Pool to free block when all allocated strings associated with block leave scope", pool.countFor(TEST_STRING_1) == 0);


    // Verify that the number of free blocks decreases when a new PoolString is allocated
    EXPECT("Pool to correctly track available blocks before an allocation", pool.countAvailable() == BLOCK_COUNT);
    EXPECT("Pool to correctly track reserved blocks before an allocation", pool.countReserved() == 0);
    PoolString testInstance = pool.findOrCreate(TEST_STRING_1);
    EXPECT("Pool to correctly track available blocks after an allocation", pool.countAvailable() == 0);
    EXPECT("Pool to correctly track available blocks after an allocation", pool.countReserved() == BLOCK_COUNT);


    // Verify that reseting the pool works
    pool.reset();
    EXPECT("Pool to correctly reset", pool.countAvailable() == BLOCK_COUNT && pool.countReserved() == 0);


    // Verify that the number of free blocks increases when a PoolString is deallocated
    PoolString testLongInstance = pool.findOrCreate(SUPER_LONG_STRING);
    EXPECT("Pool to fail to allocate a PoolString that exceeds the block size", testLongInstance == nullptr && pool.countAvailable() == BLOCK_COUNT && pool.countReserved() == 0);


    // Fill the pool with strings should assert if capacity is surpassed
    try
    {
        PoolString first = pool.findOrCreate(TEST_STRING_1);
        PoolString second = pool.findOrCreate(TEST_STRING_2);
    }
    catch (PoolOutOfMemory error)
    {
        EXPECT("Pool to fail to allocate a new PoolString when the pool is full", pool.countAvailable() == BLOCK_COUNT && pool.countReserved() == 0);
    }

    std::cout << "\nPress Any Key To Exit.." << std::endl;
    std::cin.get();
}
