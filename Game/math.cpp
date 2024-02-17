#include "math.h"
#include "intrinsics.h"

/*
 * C++ vector arithmatic implementation via
 * operator overloading 
 */

#if 0
Vec2& Vec2::operator+=(Vec2 v) {
    this->x = (this->x + v.x);
    this->y = (this->y + v.y);
    return *this;
}

Vec2& Vec2::operator-=(Vec2 v) {
    this->x = (this->x + (v.x * -1));
    this->y = (this->y + (v.y * -1));
    return *this;
}

Vec2& Vec2::operator*=(Vec2 v) {
    this->x = (this->x * v.x);
    this->y = (this->y * v.y);
    return *this;
}

Vec2& Vec2::operator*=(float scalar) {
    this->x = (this->x * scalar);
    this->y = (this->y * scalar);
    return *this;
}
#endif

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

void v3CrossProduct(Vec3 *rv, Vec3 v1, Vec3 v2)
{
    rv->x = (v1.y * v2.z) - (v1.z * v2.y);
    rv->y = (v1.z * v2.x) - (v1.x * v2.z);
    rv->z = (v1.x * v2.y) - (v1.y * v2.x);
}