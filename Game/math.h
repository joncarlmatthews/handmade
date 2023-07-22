#ifndef HEADER_HH_MATH
#define HEADER_HH_MATH

#include "types.h"

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

// Our standard vector is 32-bit floating point.
struct Vector2 {
    float32 x;
    float32 y;

    /**
     * Adds another vector to this instance's vector via overloading the
     * += operator.
     *
     * E.g:
     *
     * Vector2 v1 = {2.0f, 3.0f};
     * Vector2 v2 = {5.0f, 6.0f};
     *
     * v1.x = (v1.x + v2.x);
     * v1.y = (v1.y + v2.y);
     *
     * Can be written as:
     *
     * Vector2 v1 = {2.0f, 3.0f};
     * Vector2 v2 = {5.0f, 6.0f};
     *
     * v1 += v2; # functionally the same as above.
     *
     * @param v The vector to add.
    */
    Vector2& operator+=(Vector2 v);

    /**
     * Subtracys another vector from this instance's vector via overloading the
     * -= operator.
     *
     * E.g:
     *
     * Vector2 v1 = {2.0f, 3.0f};
     * Vector2 v2 = {5.0f, 6.0f};
     *
     * v1.x = (v1.x - v2.x);
     * v1.y = (v1.y - v2.y);
     *
     * Can be written as:
     *
     * Vector2 v1 = {2.0f, 3.0f};
     * Vector2 v2 = {5.0f, 6.0f};
     *
     * v1 -= v2; # functionally the same as above.
     *
     * @param v The vector to subtract.
    */
    Vector2& operator-=(Vector2 v);

    /**
     * Multiplies the instance's vector by another vector via overloading
     * the *= operator.
     *
     * E.g:
     *
     * Vector2 v1 = {2.0f, 3.0f};
     * Vector2 v2 = {5.0f, 6.0f};
     *
     * v1.x = (v1.x*v2.x);
     * v1.y = (v1.y*v2.y);
     *
     * Can be written as:
     *
     * Vector2 v1 = {2.0f, 3.0f};
     * Vector2 v2 = {5.0f, 6.0f};
     *
     * v1 *= v2; # functionally the same as above.
     *
     * @param v The vector to multiply by.
    */
    Vector2& operator*=(Vector2 v);

    /**
     * Multiplies the instance's vector by a scalar multiplier via overloading
     * the *= operator.
     *
     * E.g:
     *
     * Vector2 v = {2.0f, 3.0f};
     * 
     * float32 scalar = 10.0f;
     *
     * v.x = (scalar * v.x);
     * v.y = (scalar * v.y);
     *
     * Can be written as:
     *
     * Vector2 v = {2.0f, 3.0f};
     *
     * float32 scalar = 10.0f;
     *
     * v *= scalar;
     *
     * @param scalar The scalar multiplier.
    */
    Vector2& operator*=(float scalar);
} ;

float32 getVectorMagnitude(Vector2 v);

float32 modF32(float32 dividend, float32 divisor);

#endif