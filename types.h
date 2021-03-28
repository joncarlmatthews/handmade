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
typedef short               int16;
typedef int                 int32;
typedef long long           int64;
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long long  uint64;

typedef float               float32;
typedef double              float64;

typedef uint32              bool32; // For 0 or "> 0 I don't care" booleans

#endif