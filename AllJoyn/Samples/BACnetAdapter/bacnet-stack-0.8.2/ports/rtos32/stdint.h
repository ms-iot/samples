/* Defines the standard integer types that are used in code */
/* for the x86 processor and Borland Compiler */

#ifndef STDINT_H
#define STDINT_H

#include <stddef.h>

typedef unsigned char uint8_t;  /* 1 byte  0 to 255 */
typedef signed char int8_t;     /* 1 byte -127 to 127 */
typedef unsigned short uint16_t;        /* 2 bytes 0 to 65535 */
typedef signed short int16_t;   /* 2 bytes -32767 to 32767 */
/*typedef unsigned short long uint24_t;  // 3 bytes 0 to 16777215 */
typedef unsigned long uint32_t; /* 4 bytes 0 to 4294967295 */
typedef signed long int32_t;    /* 4 bytes -2147483647 to 2147483647 */
/* typedef signed long long   int64_t; */
/* typedef unsigned long long uint64_t; */

#endif /* STDINT_H */
