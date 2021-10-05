#ifndef HEADER_UTIL
#define HEADER_UTIL

#include "types.h"

/*
 * Helper functions to translate kibibytes, mebibytes and gibibytes
 * to bytes (IEC binary standard)
 *
 * @see https://en.wikipedia.org/wiki/Byte#Multiple-byte_units
 * @see https://www.quora.com/Is-1-GB-equal-to-1024-MB-or-1000-MB
 */
internal_func uint64 utilKibibytesToBytesStub(uint8 kibibytes);
internal_func uint64 utilMebibytesToBytesStub(uint8 mebibytes);
internal_func uint64 utilGibibytesToBytesStub(uint8 gibibytes);
internal_func uint64 utilTebibyteToBytesStub(uint8 tebibytes);

/**
 * Simple function to calculate one number as a percentage of another.
 *
 * @param float32 a What is (a) as a percentage of...
 * @param float32 b ?
 * @return float32
 */
internal_func float32 percentageOfAnotherf(float32 a, float32 b);

#endif