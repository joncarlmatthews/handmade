#ifndef HEADER_HH_MATH
#define HEADER_HH_MATH

//
// MATH.H
//==============================================================================
// Mathematical definitions and functions

#include <math.h>
#include "game_types.h"

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

// 2D vectors.
// -----------------------------------------------------------------------------

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
 * Returns the dot product of v1.v2.
 *
 * Dot product is the result of the two vector's x multiplied together added
 * to the two vector's y multiplied together.
 *
 * @param v1 The initial vector
 * @param v2 The vector to multiply by
 * @return The scalar dot product value
 */
float32 v2GetDotProduct(Vec2 a, Vec2 b);

/**
* Adds v2 to v1 and writes the resulting vector into rv
* 
* @param rv The resulting vector
* @param v1 The initial vector
* @param v2 The vector to add
*/
void v2Add(Vec2 *rv, Vec2 a, Vec2 b);

/**
 * Subtracts v2 from v1 and writes the resulting vector into rv
 * 
 * @param rv The resulting vector
 * @param v1 The initial vector
 * @param v2 The vector to subtract
 */
void v2Subtract(Vec2 *rv, Vec2 a, Vec2 b);

/**
* Multiplies v1 by scalar and writes the resulting vector into rv
* 
* @param rv The resulting vector
* @param v1 The initial vector
* @param scalar The amount to multiply the vector by
*/
void v2ScalarMultiply(Vec2 *rv, Vec2 a, float32 scalar);

/**
 * Calculates a vector that is perpendicular to vector v and writes the result
 * to vector rv
 *
 * @param rv The resulting vector 
 * @param v1 Vector to base calculation off of
 * @return void
*/
void v2PerpendicularVector(Vec2 *rv, Vec2 v);

// 3D vectors.
// -----------------------------------------------------------------------------

/**
 * Standard 3D vector. Our standard vector is 32-bit floating point.
 */
typedef struct Vec3 {
    float32 x;
    float32 y;
    float32 z;
} Vector3;

/**
 * Creates a 3D vector from the cross product of 3D vector A and 3D vector B
 * 
 * @param v1 
 * @param v2 
 * @return The new vector
*/
Vec3 v3VectorFromCrossProduct(Vec3 a, Vec3 b);

/**
 * Computes the magnitude of vector v
 *
 * @param v vector
 * @return magnitude
*/
float32 v3GetMagnitude(Vec3 v);

/**
 * Returns the dot product of v1.v2
 *
 * @param v1 The initial vector
 * @param v2 The vector to multiply by
 * @return The scalar dot product value
 */
float32 v3GetDotProduct(Vec3 a, Vec3 b);

/**
* Adds v2 to v1 and writes the resulting vector into rv
*
* @param rv The resulting vector
* @param v1 The initial vector
* @param v2 The vector to add
*/
void v3Add(Vec3 *rv, Vec3 a, Vec3 b);

/**
 * Subtracts v2 from v1 and writes the resulting vector into rv
 *
 * @param rv The resulting vector
 * @param v1 The initial vector
 * @param v2 The vector to subtract
 */
void v3Subtract(Vec3 *rv, Vec3 a, Vec3 b);

/**
* Multiplies v1 by scalar and writes the resulting vector into rv
*
* @param rv The resulting vector
* @param v1 The initial vector
* @param scalar The amount to multiply the vector by
*/
void v3ScalarMultiply(Vec3 *rv, Vec3 a, float32 scalar);

#endif