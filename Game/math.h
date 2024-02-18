#ifndef HEADER_HH_MATH
#define HEADER_HH_MATH

#include <math.h>
#include "game_types.h"

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

/**
 * Standard 2D vector. Our standard vector is 32-bit floating point.
 */
struct Vec2 {
    float32 x;
    float32 y;
};

/**
 * Computes the magnitude of vector v
 * 
 * @param v vector
 * @return magnitude 
*/
float32 v2GetMagnitude(Vec2 v);

/**
* Adds v2 to v1 and writes the resulting vector into rv
* 
* @param rv The resulting vector
* @param v1 The initial vector
* @param v2 The vector to add
*/
void v2Add(Vec2 *rv, Vec2 v1, Vec2 v2);

/**
 * Subtracts v2 from v1 and writes the resulting vector into rv
 * 
 * @param rv The resulting vector
 * @param v1 The initial vector
 * @param v2 The vector to subtract
 */
void v2Subtract(Vec2 *rv, Vec2 v1, Vec2 v2);

/**
* Multiplies v1 by scalar and writes the resulting vector into rv
* 
* @param rv The resulting vector
* @param v1 The initial vector
* @param scalar The amount to multiply the vector by
*/
void v2ScalarMultiply(Vec2 *rv, Vec2 v1, float32 scalar);

/**
 * Returns the dot product of v1.v2
 * 
 * @param v1 The initial vector
 * @param v2 The vector to multiply by
 * @return The scalar dot product value
 */
float32 v2GetDotProduct(Vec2 v1, Vec2 v2);

/**
 * Calculates a vector that is perpendicular to vector v and writes the result
 * to vector rv
 *
 * @param rv The resulting vector 
 * @param v1 Vector to base calculation off of
 * @return void
*/
void v2PerpendicularVector(Vec2 *rv, Vec2 v);

/**
 * Standard 3D vector. Our standard vector is 32-bit floating point.
 */
typedef struct Vec3 {
    float32 x;
    float32 y;
    float32 z;
} Vector3;

/**
 * Creates a 3D vector from the cross product of 3D vector v1 and 3D vector v2
 * 
 * @param v1 
 * @param v2 
 * @return The new vector
*/
Vec3 v3VectorFromCrossProduct(Vec3 v1, Vec3 v2);

#endif