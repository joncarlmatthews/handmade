#include "global_utility.h"

/**
* 1 KiB = 1,024 bytes
*/
uint64 utilKibibytesToBytes(uint8 kibibytes)
{
    return (uint64)((uint64)1024 * (uint64)kibibytes);
}

/**
* 1 MiB = 1,048,576 bytes
*/
uint64 utilMebibytesToBytes(uint8 mebibytes)
{
    return (uint64)(((uint64)1024 * utilKibibytesToBytes(1)) * mebibytes);
}

/**
* 1 GiB = 1,073,741,824 bytes
*/
uint64 utilGibibytesToBytes(uint8 gibibytes)
{
    return (uint64)(((uint64)1024 * utilMebibytesToBytes(1)) * gibibytes);
}

uint64 utilTebibyteToBytes(uint8 tebibytes)
{
    return (uint64)(((uint64)1024 * utilGibibytesToBytes(1)) * tebibytes);
}

/**
 * Greatest common divisor
 */
uint32 gcd(uint32 a, uint32 b)
{
    if (b == 0) {
        return a;
    }
    else {
        return gcd(b, a % b);
    }
}