#include "util.h"

internal_func uint64 utilKibibytesToBytes(uint8 kibibytes)
{
    return (uint64)((uint64)1024 * (uint64)kibibytes);
}

internal_func uint64 utilMebibytesToBytes(uint8 mebibytes)
{
    return (uint64)(((uint64)1024 * utilKibibytesToBytes(1)) * mebibytes);
}
internal_func uint64 utilGibibytesToBytes(uint8 gibibytes)
{
    return (uint64)(((uint64)1024 * utilMebibytesToBytes(1)) * gibibytes);
}
internal_func uint64 utilTebibyteToBytes(uint8 tebibytes)
{
    return (uint64)(((uint64)1024 * utilGibibytesToBytes(1)) * tebibytes);
}

internal_func float32 percentageOfAnotherf(float32 a, float32 b)
{
    if (b == 0) {
        return 0;
    }

    float32 fract = (a / b);
    return (fract * 100.0f);
}

internal_func void utilConcatStringsA(char *source1,
                                       uint32 source1Length,
                                       char *source2,
                                       uint32 source2Length,
                                       char *dest,
                                       uint32 destLength)
{
    uint32 runningIndex = 0;
    for (uint32 i = 0; i < source1Length; i++) {
        if (source1[i] == '\0') {
            break;
        }
        if (runningIndex >= (destLength - 1)) {
            break;
        }
        dest[i] = source1[i];
        runningIndex++;
    }

    for (uint32 i = 0; i < source2Length; i++) {
        if (source2[i] == '\0') {
            break;
        }
        if (runningIndex >= (destLength - 1)) {
            break;
        }
        dest[runningIndex] = source2[i];
        runningIndex++;
    }
}

internal_func void utilConcatStringsW(wchar_t *source1,
                                       uint32 source1Length,
                                       wchar_t *source2,
                                       uint32 source2Length,
                                       wchar_t *dest,
                                       uint32 destLength)
{
    uint32 runningIndex = 0;
    for (uint32 i = 0; i < source1Length; i++) {
        if (source1[i] == '\0') {
            break;
        }
        if (runningIndex >= (destLength - 1)) {
            break;
        }
        dest[i] = source1[i];
        runningIndex++;
    }

    for (uint32 i = 0; i < source2Length; i++) {
        if (source2[i] == '\0') {
            break;
        }
        if (runningIndex >= (destLength - 1)) {
            break;
        }
        dest[runningIndex] = source2[i];
        runningIndex++;
    }
}

internal_func void utilWideCharToChar(wchar_t *wideCharArr,
                                       uint32 wideCharLength,
                                       char *charArr,
                                       uint32 charLength)
{
    for (uint32 x = 0; x < wideCharLength; x++) {
        charArr[x] = (char)wideCharArr[x];
        if ('\0' == wideCharArr[x]) {
            break;
        }
        if (x >= (charLength - 1)) {
            break;
        }
    }
}

internal_func inline uint32 truncateU8(float32 f)
{
    return (uint8)f;
}

internal_func inline uint32 truncateU32(float32 f)
{
    return (uint32)f;
}