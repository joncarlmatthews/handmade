#include "math.h"
#include "intrinsics.h"

// 2D vectors.
// -----------------------------------------------------------------------------

float32 v2GetMagnitude(Vec2 v)
{
    return intrin_sqrtf((v.x * v.x) + (v.y * v.y));
}

float32 v2GetDotProduct(Vec2 a, Vec2 b)
{
    return ((a.x * b.x) + (a.y * b.y));
}

void v2Add(Vec2 *rv, Vec2 a, Vec2 b)
{
    rv->x = (a.x + b.x);
    rv->y = (a.y + b.y);
}

void v2Subtract(Vec2 *rv, Vec2 a, Vec2 b)
{
    rv->x = (a.x - b.x);
    rv->y = (a.y - b.y);
}

void v2ScalarMultiply(Vec2 *rv, Vec2 a, float32 scalar)
{
    rv->x = (a.x * scalar);
    rv->y = (a.y * scalar);
}

void v2PerpendicularVector(Vec2 *rv, Vec2 v)
{
    rv->x = v.y;
    rv->y = (v.x)*-1;
}

// 3D vectors.
// -----------------------------------------------------------------------------

Vec3 v3VectorFromCrossProduct(Vec3 a, Vec3 b)
{
    Vec3 v = {0};
    v.x = (a.y * b.z) - (a.z * b.y);
    v.y = (a.z * b.x) - (a.x * b.z);
    v.z = (a.x * b.y) - (a.y * b.x);
    return v;
}

float32 v3GetMagnitude(Vec3 v)
{
    return intrin_sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

float32 v3GetDotProduct(Vec3 a, Vec3 b)
{
    return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z));
}

void v3Add(Vec3 *rv, Vec3 a, Vec3 b)
{
    rv->x = a.x + b.x;
    rv->y = a.y + b.y;
    rv->z = a.z + b.z;
}

void v3Subtract(Vec3 *rv, Vec3 a, Vec3 b)
{
    rv->x = (a.x - b.x);
    rv->y = (a.y - b.y);
    rv->z = (a.z - b.z);
}

void v3ScalarMultiply(Vec3 *rv, Vec3 a, float32 scalar)
{
    rv->x = (a.x * scalar);
    rv->y = (a.y * scalar);
    rv->z = (a.z * scalar);
}