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
                                       UINT source1Length,
                                       char *source2,
                                       UINT source2Length,
                                       char *dest,
                                       UINT destLength)
{
    UINT runningIndex = 0;
    for (UINT i = 0; i < source1Length; i++) {
        if (source1[i] == '\0') {
            break;
        }
        if (runningIndex >= (destLength - 1)) {
            break;
        }
        dest[i] = source1[i];
        runningIndex++;
    }

    for (UINT i = 0; i < source2Length; i++) {
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
                                       UINT source1Length,
                                       wchar_t *source2,
                                       UINT source2Length,
                                       wchar_t *dest,
                                       UINT destLength)
{
    UINT runningIndex = 0;
    for (UINT i = 0; i < source1Length; i++) {
        if (source1[i] == '\0') {
            break;
        }
        if (runningIndex >= (destLength - 1)) {
            break;
        }
        dest[i] = source1[i];
        runningIndex++;
    }

    for (UINT i = 0; i < source2Length; i++) {
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
                                       UINT wideCharLength,
                                       char *charArr,
                                       UINT charLength)
{
    for (UINT x = 0; x < wideCharLength; x++) {
        charArr[x] = (char)wideCharArr[x];
        if ('\0' == wideCharArr[x]) {
            break;
        }
        if (x >= (charLength - 1)) {
            break;
        }
    }
}