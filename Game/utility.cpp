#include "utility.h"

uint64 utilKibibytesToBytes(uint8 kibibytes)
{
    return (uint64)((uint64)1024 * (uint64)kibibytes);
}

uint64 utilMebibytesToBytes(uint8 mebibytes)
{
    return (uint64)(((uint64)1024 * utilKibibytesToBytes(1)) * mebibytes);
}
uint64 utilGibibytesToBytes(uint8 gibibytes)
{
    return (uint64)(((uint64)1024 * utilMebibytesToBytes(1)) * gibibytes);
}
uint64 utilTebibyteToBytes(uint8 tebibytes)
{
    return (uint64)(((uint64)1024 * utilGibibytesToBytes(1)) * tebibytes);
}

float32 percentageOfAnotherf(float32 a, float32 b)
{
    if (b <= 0) {
        return 0.0f;
    }

    float32 fract = (a / b);
    return (fract * 100.0f);
}

void utilConcatStringsA(char *source1,
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

void utilConcatStringsW(wchar_t *source1,
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

void utilWideCharToChar(wchar_t *wideCharArr,
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

inline uint32 truncateU8(float32 f)
{
    return (uint8)f;
}

inline uint32 truncateU32(float32 f)
{
    return (uint32)f;
}

int32 modulo(int32 dividend, uint32 divisor)
{
    if (dividend < 0) {
        dividend = (divisor + dividend);
    }

    uint32 res = (dividend / divisor);
    res = (res * divisor);
    return (dividend - res);
}