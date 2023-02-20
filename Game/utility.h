#ifndef HEADER_HH_UTILITIES
#define HEADER_HH_UTILITIES

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

/**
 * Simple function to calculate one number as a percentage of another.
 *
 * @param float32 a What is (a) as a percentage of...
 * @param float32 b ?
 * @return float32
 */
float32 percentageOfAnotherf(float32 a, float32 b);

/**
 * Concatenates two wide char strings
 *
 */
void utilConcatStringsW(wchar_t *source1,
                        uint32 source1Length,
                        wchar_t *source2,
                        uint32 source2Length,
                        wchar_t *dest,
                        uint32 destLength);

/**
 * Concatenates two char strings
 *
 */
void utilConcatStringsA(char *source1,
                        uint32 source1Length,
                        char *source2,
                        uint32 source2Length,
                        char *dest,
                        uint32 destLength);

/**
 * Converts a wide char to a regular char
 *
 */
void utilWideCharToChar(wchar_t *wideCharArr,
                        uint32 wideCharLength,
                        char *charArr,
                        uint32 charLength);

int32 modulo(int32 dividend, uint32 divisor);

uint32 u32RoundUpDivide(uint32 dividend, uint32 divisor);

int32 i32RoundUpDivide(int32 dividend, int32 divisor);

#endif