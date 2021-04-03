#ifndef HEADER_GC_TYPES
#define HEADER_GC_TYPES

// Typedefs that specify exact-width integer types for increased code portability.
/*
 * char:        (1)     int8  / uint8   (-128 127)          (0 255)
 * short:       (2)     int16 / uint16  (-32,768 32,767)    (0 65,536)
 * int (long):  (4)     int32 / uint32  (-2.1bn to 2.1bn)   (0 to 4.2bn)
 * long long:   (8)     int64 / uint64  (-9qn 9qn)          (0-18qn)
 */
typedef signed char         int8;
typedef signed short        int16;
typedef signed int          int32;
typedef signed long long    int64;

typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long long  uint64;

typedef float               float32;
typedef double              float64;

typedef int8                bool8;
typedef int16               bool16;
typedef int32               bool32;
typedef int64               bool64;

#endif