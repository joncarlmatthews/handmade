#include "math.h"
#include "intrinsics.h"

/*
 * C++ vector arithmatic implementation via
 * operator overloading 
 */

#if 0
Vector2& Vector2::operator+=(Vector2 v) {
    this->x = (this->x + v.x);
    this->y = (this->y + v.y);
    return *this;
}

Vector2& Vector2::operator-=(Vector2 v) {
    this->x = (this->x + (v.x * -1));
    this->y = (this->y + (v.y * -1));
    return *this;
}

Vector2& Vector2::operator*=(Vector2 v) {
    this->x = (this->x * v.x);
    this->y = (this->y * v.y);
    return *this;
}

Vector2& Vector2::operator*=(float scalar) {
    this->x = (this->x * scalar);
    this->y = (this->y * scalar);
    return *this;
}
#endif

void vector2Add(Vector2 *v1, Vector2 v2)
{
    v1->x = (v1->x + v2.x);
    v1->y = (v1->y + v2.y);
}

void vector2Subtract(Vector2 *rv, Vector2 v1, Vector2 v2)
{
    rv->x = (v1.x + (v2.x * -1));
    rv->y = (v1.y + (v2.y * -1));
}

void vector2Multiply(Vector2 *v1, Vector2 v2)
{
    v1->x = (v1->x * v2.x);
    v1->y = (v1->y * v2.y);
}

void vector2MultiplyScalar(Vector2 *v1, float32 scalar)
{
    v1->x = (v1->x * scalar);
    v1->y = (v1->y * scalar);
}

float32 getVectorMagnitude(Vector2 v)
{
    return intrin_sqrtf((v.x * v.x) + (v.y * v.y));
}