#ifndef HEADER_HH_MEMORY
#define HEADER_HH_MEMORY

#include "types.h"

typedef struct GameMemoryRegion
{
    // Address
    void *bytes;

    // Total size in bytes of the memory block
    sizet sizeInBytes;

    // How many bytes used and how many are left free?
    sizet bytesUsed;
    sizet bytesFree;

} GameMemoryRegion;

typedef struct GameMemoryBlock
{
    // The starting address for the memory block
    // 8 or 4 bytes in size (x64/x86). uint8 to step 1 address at a time
    uint8 *startingAddress;

    // The last address that was resrved to within the memory block.
    // 8 or 4 bytes in size (x64/x86). uint8 to step 1 address at a time
    uint8 *lastAddressReserved;

    // The total size in bytes that the memory block can allocate
    sizet totalSizeInBytes;

    // How many bytes used and how many are left free?
    sizet bytesUsed;
    sizet bytesFree;
} GameMemoryBlock;

/**
* Initialises a game memory block with a starting address and the
* maximum number of bytes that the bock can fill up to
* 
* @param *memoryBlock           Address of memory block to write to
* @param *startingAddress       Address of the first piece of memory that can be written to
* @param maximumSizeInBytes     Maximum size in bytes that the memory block can take up
*/
void initGameMemoryBlock(GameMemoryRegion memoryRegion,
                            GameMemoryBlock *memoryBlock,
                            uint8 *startingAddress,
                            sizet maximumSizeInBytes);

/**
 * "Reserves" a region of the GameMemoryBlock with enough space for a given type
 * by keeping track of the starting address and size of the data type within
 * the GameMemoryBlock. Doesn't write to any data. Returns the starting memory
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
#define gameMemoryBlockReserveStruct(memoryRegion, memoryBlockPtr, type) (type *)gameMemoryBlockReserveType_(memoryRegion, memoryBlockPtr, sizeof(type), 1)

/* 
 * Reserves a region for an array with n number of elements
 * 
 * @param memoryBlockPtr    Pointer to relevant memory block to read from/write to
 * @param type              The type of the array
 * @param noOfElements      The size of the array
 * @return void*            The starting memory address of the reserved region.
 *
 */
#define gameMemoryBlockReserveArray(memoryRegion, memoryBlockPtr, type, noOfElements) (type *)gameMemoryBlockReserveType_(memoryRegion, memoryBlockPtr, sizeof(type), noOfElements)

/*
 * Macros so we can reserve regions for different data types
 * 
 * @param memoryBlock   Pointer to relevant memory block to read from/write to
 * @param typeSize      Size in bytes of the type to reserve
 * @param noOfTypes     How many of the types to reserve
 * @return void*        The starting memory address of the reserved region.
*/
void *gameMemoryBlockReserveType_(GameMemoryRegion *memoryRegion,
                                    GameMemoryBlock *memoryBlock,
                                    sizet typeSize,
                                    sizet noOfTypes);

#endif