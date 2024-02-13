#ifndef HEADER_HH_MATH
#define HEADER_HH_MATH

#include <math.h>
#include "game_types.h"

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

// Our standard vector is 32-bit floating point.
struct Vector2 {
    float32 x;
    float32 y;

#if 0

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
#endif

};

/**
 * Computes the magnitude of vector v
 * 
 * @param v vector
 * @return magnitude 
*/
float32 getVectorMagnitude(Vector2 v);

/**
* Adds v2 to v1 and writes the resulting vector into rv
* 
* @param rv The resulting vector
* @param v1 The initial vector
* @param v2 The vector to add
*/
void vector2Add(Vector2 *rv, Vector2 v1, Vector2 v2);

/**
 * Subtracts v2 from v1 and writes the resulting vector into rv
 * 
 * @param rv The resulting vector
 * @param v1 The initial vector
 * @param v2 The vector to subtract
 */
void vector2Subtract(Vector2 *rv, Vector2 v1, Vector2 v2);

/**
* Multiplies v1 by scalar and writes the resulting vector into rv
* 
* @param rv The resulting vector
* @param v1 The initial vector
* @param scalar The amount to multiply the vector by
*/
void vector2MultiplyScalar(Vector2 *rv, Vector2 v1, float32 scalar);

/**
 * Multiplies v1 by v2 and writes the resulting vector into rv
 * 
 * @param rv The resulting vector
 * @param v1 The initial vector
 * @param v2 The vector to multiply by
 */
void vector2Multiply(Vector2 *rv, Vector2 v1, Vector2 v2);

#endif