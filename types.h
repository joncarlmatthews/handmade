#ifndef HEADER_GC_TYPES
#define HEADER_GC_TYPES

// Typedefs that specify exact-width integer types for increased code portability.
/*
 * char:                                (1)     int8   (-128 127)
 * short:                               (2)     int16  (-32,768 32,767)
 * int (aka long):                      (4)     int32  (-2.1bn to 2.1bn) 2,147,483,647 (2GiB in byte count)
 * long long:                           (8)     int64  (-9qn 9qn)
 * 
 * unsigned char:                       (1)     uint8   (0 255)
 * unsigned short:                      (2)     uint16  (0 65,536)
 * unsigned int (aka unsigned long):    (4)     uint32  (0 to 4.2bn) 4,294,967,295 (4GiB in byte count)
 * unsigned long long:                  (8)     uint64  (0 to 18qn)
 */

typedef signed char         int8;   // 1 byte
typedef signed short        int16;  // 2 bytes
typedef signed int          int32;  // 4 bytes (aka long)
typedef signed long long    int64;  // 8 bytes

typedef unsigned char       uint8;   // 1 byte
typedef unsigned short      uint16;  // 2 bytes
typedef unsigned int        uint32;  // 4 bytes (aka long)
typedef unsigned long long  uint64;  // 8 bytes

typedef float               float32;  // 4 bytes
typedef double              float64;  // 8 bytes

typedef int8                bool8;
typedef int16               bool16;
typedef int32               bool32;
typedef int64               bool64;

#endif