#ifndef HEADER_HH_MEMORY
#define HEADER_HH_MEMORY

#include "types.h"

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
* @brief Initialises a game memory block with a starting address and the
* maximum number of bytes that the bock can fill up to
* 
* @param memoryBlock 
* @param startingAddress 
* @param maximumSizeInBytes 
*/
void initGameMemoryBlock(GameMemoryBlock *memoryBlock,
                            uint8 *startingAddress,
                            sizet maximumSizeInBytes);

/**
* "Reserves" a region of the memory block with enough space for a given type
* by keeping track of the starting address and size of the data type within
* the GameMemoryBlock. Doesn't write to any data. Returns the starting memory
* address of the reserved region.
*
* Macros so we can reserve regions for different data types
* 
* @param memoryBlock   Pointer to relevant memory block to read from/write to
* @param typeSize      Size in bytes of the type to reserve
* @param noOfTypes     How many of the types to reserve
* @return void*        The starting memory address of the reserved region.
*/
#define gameMemoryBlockReserveStruct(memoryBlockPtr, type) (type *)gameMemoryBlockReserveType_(memoryBlockPtr, sizeof(type), 1)
#define gameMemoryBlockReserveArray(memoryBlockPtr, type, noOfElements) (type *)gameMemoryBlockReserveType_(memoryBlockPtr, sizeof(type), noOfElements)
void *gameMemoryBlockReserveType_(GameMemoryBlock *memoryBlock, sizet typeSize, sizet noOfTypes);

#endif