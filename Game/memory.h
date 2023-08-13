#ifndef HEADER_HH_MEMORY
#define HEADER_HH_MEMORY

#include "game_types.h"

/**
 * @NOTE(JM)
 * Allocated memory contains Memory Regions. Memory Regions contain Memory Blocks
 */

/**
 * Memory Regions are high-level logical sections of the platform layer's memory.
 * E.g. "permanent storage" or "transient storage"
*/
typedef struct MemoryRegion
{
    // Address
    void *bytes;

    // Total size in bytes of the memory block
    sizet sizeInBytes;

    // How many bytes used and how many are left free?
    sizet bytesUsed;
    sizet bytesFree;

} MemoryRegion;

/**
 * Memory Blocks are used to reserve sections of a Memory Region for a particular
 * use. E.g. use 1MB of this Memory Region for enemies. Or, use 5MB of this Memory
 * Region for tile data. Etc etc.
 *
 * Using blocks of memory allows you to carve up the Memory Region into sections
 * and keeps track of the starting and ending address.
*/
typedef struct MemoryBlock
{
    // The starting address for the memory block
    // 8 or 4 bytes in size (x64/x86). uint8 to step 1 address at a time
    uint8 *startingAddress;

    // The last possible address for the memory block
    // 8 or 4 bytes in size (x64/x86). uint8 to step 1 address at a time
    uint8 *endingAddress;

    // The last address that was reserved to within the memory block.
    // 8 or 4 bytes in size (x64/x86). uint8 to step 1 address at a time
    uint8 *lastAddressReserved;

    // The total size in bytes that the memory block can allocate
    sizet totalSizeInBytes;

    // How many bytes used and how many are left free?
    sizet bytesUsed;
    sizet bytesFree;
} MemoryBlock;

/**
* Initialises a game memory block with a starting address and the
* maximum number of bytes that the block can fill up to
* 
* @param *memoryBlock           Address of memory block to write to
* @param *startingAddress       Address of the first piece of memory that can be written to
* @param maximumSizeInBytes     Maximum size in bytes that the memory block can take up
*/
void memoryRegionReserveBlock(MemoryRegion memoryRegion,
                                MemoryBlock *memoryBlock,
                                uint8 *startingAddress,
                                sizet maximumSizeInBytes);

/**
 * "Reserves" a region of the MemoryBlock with enough space for a given type
 * by keeping track of the starting address and size of the data type within
 * the MemoryBlock. Doesn't write to any data. Returns the starting memory
 * address of the reserved region.
 *
 */

/* 
 * Reserves a region for a single struct
 * 
 * @param memoryBlockPtr    Pointer to relevant memory block to read from/write to
 * @param type              The type of the struct
 * @return void*            The starting memory address of the reserved region.
 *
 */
#define memoryBlockReserveStruct(memoryRegion, memoryBlockPtr, type) (type *)memoryBlockReserveType_(memoryRegion, memoryBlockPtr, sizeof(type), 1)

/* 
 * Reserves a region for an array with n number of elements
 * 
 * @param memoryBlockPtr    Pointer to relevant memory block to read from/write to
 * @param type              The type of the array
 * @param noOfElements      The size of the array
 * @return void*            The starting memory address of the reserved region.
 *
 */
#define memoryBlockReserveArray(memoryRegion, memoryBlockPtr, type, noOfElements) (type *)memoryBlockReserveType_(memoryRegion, memoryBlockPtr, sizeof(type), noOfElements)

/*
 * Macros so we can reserve regions for different data types
 * 
 * @param memoryBlock   Pointer to relevant memory block to read from/write to
 * @param typeSize      Size in bytes of the type to reserve
 * @param noOfTypes     How many of the types to reserve
 * @return void*        The starting memory address of the reserved region.
*/
void *memoryBlockReserveType_(MemoryRegion *memoryRegion,
                                MemoryBlock *memoryBlock,
                                sizet typeSize,
                                sizet noOfTypes);

#endif