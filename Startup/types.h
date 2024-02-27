#ifndef HEADER_SHARED_TYPES
#define HEADER_SHARED_TYPES

//
// TYPES.H
//==============================================================================
// Use type definitions that can be shared across the platform and game layers.


// Useful definitions from the C Standard Library.
// -----------------------------------------------------------------------------
#include <stddef.h> // size_t
#include <stdbool.h> // true, false, bool


// Custom typedefs that specify exact-width types for increased code portability.
// -----------------------------------------------------------------------------

/*
 * char:                                (1)     int8   (-128 127)
 * short:                               (2)     int16  (-32,768 32,767)
 * int (aka long):                      (4)     int32  (-2.1bn to 2.1bn) 2,147,483,647 (2GiB in byte count)
 * long long:                           (8)     int64  (-9qn 9qn)
 *
 * unsigned char:                       (1)     uint8   (0 255)
 * unsigned short:                      (2)     uint16  (0 65,535)
 * unsigned int (aka unsigned long):    (4)     uint32  (0 to 4.2bn) 4,294,967,295 (4GiB in byte count)
 * unsigned long long:                  (8)     uint64  (0 to 18qn)
 */

typedef signed char         int8;       // 1 byte
typedef signed short        int16;      // 2 bytes
typedef signed int          int32;      // 4 bytes (aka long)
typedef signed long long    int64;      // 8 bytes

typedef unsigned char       uint8;      // 1 byte
typedef unsigned short      uint16;     // 2 bytes
typedef unsigned int        uint32;     // 4 bytes (aka unsigned long)
typedef unsigned long long  uint64;     // 8 bytes

typedef float               float32;    // 4 bytes
typedef double              float64;    // 8 bytes

typedef size_t              sizet;      // Guaranteed to be able to represent
                                        // the size of the largest object that
                                        // can exist in the memory of the system

typedef int8                bool8;
typedef int16               bool16;
typedef int32               bool32;
typedef int64               bool64;

#endif