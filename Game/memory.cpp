#include "memory.h"

void memoryRegionReserveBlock(MemoryRegion memoryRegion,
                            MemoryBlock *memoryBlock,
                            uint8 *startingAddress,
                            sizet maximumSizeInBytes)
{
    assert(maximumSizeInBytes <= memoryRegion.bytesFree);

    memoryBlock->startingAddress        = startingAddress;
    memoryBlock->endingAddress          = (startingAddress + (maximumSizeInBytes -1));
    memoryBlock->lastAddressReserved    = 0;
    memoryBlock->totalSizeInBytes       = maximumSizeInBytes;
    memoryBlock->bytesUsed              = 0;
    memoryBlock->bytesFree              = maximumSizeInBytes;
}

void *memoryBlockReserveType_(MemoryRegion *memoryRegion,
                                    MemoryBlock *memoryBlock,
                                    sizet typeSize,
                                    sizet noOfTypes)
{
    assert(noOfTypes > 0);
    assert(typeSize > 0);

    // How many bytes are we reserving?
    sizet bytesToReserve = (typeSize * noOfTypes);

    // Check that the size requested fits within what's free
    assert(bytesToReserve <= memoryRegion->bytesFree);
    assert(bytesToReserve <= memoryBlock->bytesFree);

    // What's the reserved starting address within the memory block?
    uint8 *reservedStartingAddress;

    if (memoryBlock->lastAddressReserved) {
        reservedStartingAddress = (memoryBlock->lastAddressReserved + 1);
    } else {
        // First time weve allocated within this memory block...
        reservedStartingAddress = memoryBlock->startingAddress;
    }

    // Update the bytes used/free
    memoryBlock->bytesUsed = (memoryBlock->bytesUsed + bytesToReserve);
    memoryBlock->bytesFree = (memoryBlock->bytesFree - bytesToReserve);

    // Update the memory block's last reserved address
    memoryBlock->lastAddressReserved = (reservedStartingAddress + (bytesToReserve -1));

    // Check that the total bytes now used within this memory block don't overrun
    // the total available.
    assert(memoryBlock->bytesUsed <= memoryBlock->totalSizeInBytes);

    // Update our memory region's meta data
    memoryRegion->bytesUsed = (memoryRegion->bytesUsed + bytesToReserve);
    memoryRegion->bytesFree = (memoryRegion->bytesFree - bytesToReserve);

    return (void *)reservedStartingAddress;
}