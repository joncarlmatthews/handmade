#ifndef HEADER_HH_UTIL
#define HEADER_HH_UTIL

#include "types.h"

/*
 * Helper functions to translate kibibytes, mebibytes and gibibytes
 * to bytes (IEC binary standard)
 *
 * @see https://en.wikipedia.org/wiki/Byte#Multiple-byte_units
 * @see https://www.quora.com/Is-1-GB-equal-to-1024-MB-or-1000-MB
 */
internal_func uint64 utilKibibytesToBytes(uint8 kibibytes);
internal_func uint64 utilMebibytesToBytes(uint8 mebibytes);
internal_func uint64 utilGibibytesToBytes(uint8 gibibytes);
internal_func uint64 utilTebibyteToBytes(uint8 tebibytes);

/**
 * Simple function to calculate one number as a percentage of another.
 *
 * @param float32 a What is (a) as a percentage of...
 * @param float32 b ?
 * @return float32
 */
internal_func float32 percentageOfAnotherf(float32 a, float32 b);

/**
 * Concatenates two wide char strings
 *
 */
internal_func void utilConcatStringsW(wchar_t* source1, UINT source1Length, wchar_t* source2, UINT source2Length, wchar_t* dest, UINT destLength);

/**
 * Concatenates two char strings
 *
 */
internal_func void utilConcatStringsA(char* source1, UINT source1Length, char* source2, UINT source2Length, char* dest, UINT destLength);

/**
 * Converts a wide char to a regular char
 *
 */
internal_func void utilWideCharToChar(wchar_t* wideCharArr, UINT wideCharLength, char* charArr, UINT charLength);

#endif