#include "math.h"
#include "intrinsics.h"

// Utilities
// -----------------------------------------------------------------------------

uint32 gcd(uint32 a, uint32 b)
{
    if(b == 0){
        return a;
    } else{
        return gcd(b, a % b);
    }
}

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

float32 v2GetCrossProduct(Vec2 a, Vec2 b)
{
    return (a.x * b.y - a.y * b.x);
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

void v2Normal(Vec2 *rv, Vec2 v)
{
    float32 mag = v2GetMagnitude(v);
    rv->x = v.x / mag;
    rv->y = v.y / mag;
}

void v2Rotate(Vec2 *rv, Vec2 v, float32 radians)
{
    rv->x = (v.x * intrin_cosf(radians)) - (v.y * intrin_sinf(radians));
    rv->y = (v.x * intrin_sinf(radians)) + (v.y * intrin_cosf(radians));
}

// 3D vectors.
// -----------------------------------------------------------------------------

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

void v3CrossProduct(Vec3 *rv, Vec3 a, Vec3 b)
{
    rv->x = (a.y * b.z) - (a.z * b.y);
    rv->y = (a.z * b.x) - (a.x * b.z);
    rv->z = (a.x * b.y) - (a.y * b.x);
}

void v3Normal(Vec3 *rv, Vec3 v)
{
    float32 mag = v3GetMagnitude(v);
    rv->x = v.x / mag;
    rv->y = v.y / mag;
    rv->z = v.z / mag;
}