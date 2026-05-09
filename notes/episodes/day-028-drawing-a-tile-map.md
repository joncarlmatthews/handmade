# Day 028 - Drawing a Tile Map

Variables are written to starting from the right most bit, and move left one-bit at a time. Any bits not set, are left as 0. E.g.

```c
uint32 testVar;
testVar = 0xFFFF;      // 00000000 00000000 11111111 11111111
testVar = 0xFFFFFF;    // 00000000 11111111 11111111 11111111
testVar = 0xFFFFFFFF;  // 11111111 11111111 11111111 11111111
```
