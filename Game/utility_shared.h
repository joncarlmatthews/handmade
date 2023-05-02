#ifndef HEADER_HH_UTILITY_SHARED
#define HEADER_HH_UTILITY_SHARED

/*
 * Utility/helper Functions that are shared across the game and platform layer
 */

#include "types.h"

/*
* Helper functions to translate kibibytes, mebibytes and gibibytes
* to bytes (IEC binary standard)
*
* @see https://en.wikipedia.org/wiki/Byte#Multiple-byte_units
* @see https://www.quora.com/Is-1-GB-equal-to-1024-MB-or-1000-MB
*/
uint64 utilKibibytesToBytes(uint8 kibibytes);
uint64 utilMebibytesToBytes(uint8 mebibytes);
uint64 utilGibibytesToBytes(uint8 gibibytes);
uint64 utilTebibyteToBytes(uint8 tebibytes);


uint32 gcd(uint32 a, uint32 b);

#endif