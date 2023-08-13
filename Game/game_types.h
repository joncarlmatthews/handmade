#ifndef HEADER_HH_GAMETYPES
#define HEADER_HH_GAMETYPES

#include "startup.h"


//
// Game types...
//====================================================
typedef struct xyint {
    int32 x;
    int32 y;
} xyint;

typedef struct xyuint {
    uint32 x;
    uint32 y;
} xyuint;

typedef struct xyzuint {
    uint32 x;
    uint32 y;
    uint32 z;
} xyzuint;

typedef struct Colour {
    float32 r; // Between 0.0f and 1.0f
    float32 g; // Between 0.0f and 1.0f
    float32 b; // Between 0.0f and 1.0f
    float32 a; // Between 0.0f and 1.0f
} Colour;

#endif