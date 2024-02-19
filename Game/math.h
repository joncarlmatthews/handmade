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
typedef struct Vec2 {
    float32 x;
    float32 y;
} Vec2;

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
 * @param a The initial vector
 * @param b  The vector to multiply by
 * @return The scalar dot product value
 */
float32 v2GetDotProduct(Vec2 a, Vec2 b);

/**
 * Returns the cross product of a and b. In 2D space, the cross product
 * is a scalar value, not a vector.
 *
 * @param a
 * @param b
 * @return The scalar dot product value
 */
float32 v2GetCrossProduct(Vec2 a, Vec2 b);

/**
 * Adds v2 to v1 and writes the resulting vector into rv
 * 
 * @param rv The resulting vector
 * @param a The initial vector
 * @param b  The vector to add
 */
void v2Add(Vec2 *rv, Vec2 a, Vec2 b);

/**
 * Subtracts v2 from v1 and writes the resulting vector into rv
 * 
 * @param rv The resulting vector
 * @param a The initial vector
 * @param b  The vector to subtract
 */
void v2Subtract(Vec2 *rv, Vec2 a, Vec2 b);

/**
 * Multiplies v1 by scalar and writes the resulting vector into rv
 * 
 * @param rv The resulting vector
 * @param a The initial vector
 * @param scalar The amount to multiply the vector by
 */
void v2ScalarMultiply(Vec2 *rv, Vec2 a, float32 scalar);

/**
 * Calculates a vector that is perpendicular to vector v and writes the result
 * to vector rv
 *
 * @param rv The resulting vector 
 * @param a Vector to base calculation off of
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
 * Computes and returns the magnitude of vector v
 *
 * @param v vector
 * @return magnitude
 */
float32 v3GetMagnitude(Vec3 v);

/**
 * Computes and returns the dot product of a.b
 *
 * @param a The initial vector
 * @param b  The vector to multiply by
 * @return The scalar dot product value
 */
float32 v3GetDotProduct(Vec3 a, Vec3 b);

/**
 * Adds vector A to vector B and writes the resulting vector into rv
 *
 * @param rv The resulting vector
 * @param a The initial vector
 * @param b  The vector to add
 */
void v3Add(Vec3 *rv, Vec3 a, Vec3 b);

/**
 * Subtracts vector A vector from B and writes the resulting vector into rv
 *
 * @param rv The resulting vector
 * @param a The initial vector
 * @param b  The vector to subtract
 */
void v3Subtract(Vec3 *rv, Vec3 a, Vec3 b);

/**
 * Multiplies vector A by scalar and writes the resulting vector into rv
 *
 * @param rv The resulting vector
 * @param a The initial vector
 * @param scalar The amount to multiply the vector by
 */
void v3ScalarMultiply(Vec3 *rv, Vec3 a, float32 scalar);

/**
 * Calculates the cross product of 3D vector A and 3D vector B and writes the
 * resulting vector into rv
 *
 * @see https://t.ly/ITA7Y
 * @see https://t.ly/WA4dP
 *
 * @param rv The resulting vector 
 * @param a
 * @param b
 */
void v3CrossProduct(Vec3 *rv, Vec3 a, Vec3 b);

float32 v3GetNormal(Vec3 v);

#endif