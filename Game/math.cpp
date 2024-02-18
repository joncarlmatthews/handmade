#include "math.h"
#include "intrinsics.h"

float32 v2GetMagnitude(Vec2 v)
{
    return intrin_sqrtf((v.x * v.x) + (v.y * v.y));
}

void v2Add(Vec2 *rv, Vec2 v1, Vec2 v2)
{
    rv->x = (v1.x + v2.x);
    rv->y = (v1.y + v2.y);
}

void v2Subtract(Vec2 *rv, Vec2 v1, Vec2 v2)
{
    rv->x = (v1.x - v2.x);
    rv->y = (v1.y - v2.y);
}

void v2ScalarMultiply(Vec2 *rv, Vec2 v1, float32 scalar)
{
    rv->x = (v1.x * scalar);
    rv->y = (v1.y * scalar);
}

float32 v2GetDotProduct(Vec2 v1, Vec2 v2)
{
    return ((v1.x * v2.x) + (v1.y * v2.y));
}

void v2PerpendicularVector(Vec2 *rv, Vec2 v)
{
    rv->x = v.y;
    rv->y = (v.x)*-1;
}

Vec3 v3VectorFromCrossProduct(Vec3 v1, Vec3 v2)
{
    Vec3 v = {0};
    v.x = (v1.y * v2.z) - (v1.z * v2.y);
    v.y = (v1.z * v2.x) - (v1.x * v2.z);
    v.z = (v1.x * v2.y) - (v1.y * v2.x);
    return v;
}