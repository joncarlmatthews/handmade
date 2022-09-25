#include "memory.h"

void initGameMemoryBlock(GameMemoryBlock *memoryBlock,
                            uint8 *startingAddress,
                            sizet maximumSizeInBytes)
{
    memoryBlock->startingAddress        = startingAddress;
    memoryBlock->lastAddressReserved    = startingAddress;
    memoryBlock->totalSizeInBytes       = maximumSizeInBytes;
    memoryBlock->bytesUsed              = 0;
    memoryBlock->bytesFree              = maximumSizeInBytes;
}

void *gameMemoryBlockReserveType_(GameMemoryBlock *memoryBlock, sizet typeSize, sizet noOfTypes)
{
    // How many bytes are we reserving?
    sizet bytesToReserve = (typeSize * noOfTypes);

    // Check that the size requested fits within what's free
    assert(bytesToReserve <= memoryBlock->bytesFree);

    // What's the reserved starting address within the memory block?
    void *reservedStartingAddress = (memoryBlock->lastAddressReserved + 1);

    // Update the bytes used/free
    memoryBlock->bytesUsed = (memoryBlock->bytesUsed + bytesToReserve);
    memoryBlock->bytesFree = (memoryBlock->bytesFree - bytesToReserve);

    // Update the memory block's last reserved address
    memoryBlock->lastAddressReserved = (memoryBlock->lastAddressReserved + bytesToReserve);

    // Check that the total bytes now used within this memory block don't overrun
    // the total available.
    assert(memoryBlock->bytesUsed <= memoryBlock->totalSizeInBytes);

    return reservedStartingAddress;
}