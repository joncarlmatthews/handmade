#include "math.h"
#include "intrinsics.h"

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

Vector2& Vector2::operator*=(float scalar) {
    scalar = scalar * 10.0f;
    this->x = (scalar * this->x);
    this->y = (scalar * this->y);
    return *this;
}

Vector2& Vector2::operator*=(Vector2 v) {
    this->x = (this->x * v.x);
    this->y = (this->y * v.y);
    return *this;
}

float32 getVectorMagnitude(Vector2 v)
{
    return intrin_sqrtf((v.x * v.x) + (v.y * v.y));
}

float32 modF32(float32 dividend, float32 divisor) {
    return (dividend - ((int)(dividend / divisor) * divisor));
}