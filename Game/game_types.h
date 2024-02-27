#ifndef HEADER_HH_GAMETYPES
#define HEADER_HH_GAMETYPES

#include "startup.h"

//
// Game types...
//====================================================
// Temp. Being replaced by vectors

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

#endif