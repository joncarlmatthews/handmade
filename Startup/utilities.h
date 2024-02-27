#ifndef HEADER_SHARED_UTILS
#define HEADER_SHARED_UTILS

//
// UTILITIES.H
//==============================================================================
// Useful, small utility functions that are shared across the platform and game
// layers

/*
 * Convert kibibytes, mebibytes and gibibytes to bytes (IEC binary standard)
 *
 * @see https://en.wikipedia.org/wiki/Byte#Multiple-byte_units
 * @see https://www.quora.com/Is-1-GB-equal-to-1024-MB-or-1000-MB
 */

/**
 * Kilobytes to bytes.
 * 
 * 1 KiB = 1,024 bytes
 */
size_t utilKibibytesToBytes(uint32 kibibytes);

/**
 * Megabytes to bytes.
 * 
 * 1 MiB = 1,048,576 bytes
 */
sizet utilMebibytesToBytes(uint32 mebibytes);

/**
 * Gigabytes to bytes.
 * 
 * 1 GiB = 1,073,741,824 bytes
 */
sizet utilGibibytesToBytes(uint32 gibibytes);

/**
 * Terabytes to bytes.
 * 
 * 1 Tb = 1,099,511,627,776 bytes
 */
sizet utilTebibyteToBytes(uint32 tebibytes);

/**
 * Greatest common divisor
 */
uint32 gcd(uint32 a, uint32 b);

#endif