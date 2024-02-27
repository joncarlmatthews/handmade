#include "startup.h"

size_t utilKibibytesToBytes(uint32 kibibytes)
{
    return (size_t)((uint32)1024 * kibibytes);
}

size_t utilMebibytesToBytes(uint32 mebibytes)
{
    return (size_t)(((uint32)1024 * utilKibibytesToBytes(1)) * mebibytes);
}

size_t utilGibibytesToBytes(uint32 gibibytes)
{
    return (size_t)(((uint32)1024 * utilMebibytesToBytes(1)) * gibibytes);
}

size_t utilTebibyteToBytes(uint32 tebibytes)
{
    return (size_t)(((uint32)1024 * utilGibibytesToBytes(1)) * tebibytes);
}

uint32 gcd(uint32 a, uint32 b)
{
    if(b == 0) {
        return a;
    }else{
        return gcd(b, a % b);
    }
}