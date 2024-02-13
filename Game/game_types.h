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

// Colour struct. Supports a hex colour or a colour that is comprised of a series
// of normalised RGBA values between 0.0 and 1.0.
// E.g. {0x336996} -or- {0.0f, 0.2f, 0.7f, 1.0f}
// To specify an RGBA colour, set the hex member to 0.0f
typedef struct Colour {
    float32 hex;
    float32 r; // Between 0.0f and 1.0f
    float32 g; // Between 0.0f and 1.0f
    float32 b; // Between 0.0f and 1.0f
    float32 a; // Between 0.0f and 1.0f
} Colour;

#endif