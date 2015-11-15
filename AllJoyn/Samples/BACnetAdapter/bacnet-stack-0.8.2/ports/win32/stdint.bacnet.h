/* Defines the standard integer types that are used in code */
/* for the x86 processor and Borland Compiler */

#ifndef _STDINT_H
#define _STDINT_H

#include <stddef.h>

typedef unsigned char uint8_t;  /* 1 byte  0 to 255 */
typedef signed char int8_t;     /* 1 byte -127 to 127 */
typedef unsigned short uint16_t;        /* 2 bytes 0 to 65535 */
typedef signed short int16_t;   /* 2 bytes -32767 to 32767 */
/*typedef unsigned short long uint24_t;  // 3 bytes 0 to 16777215 */
typedef unsigned uint32_t;      /* 4 bytes 0 to 4294967295 */
typedef int int32_t;    /* 4 bytes -2147483647 to 2147483647 */
/* typedef signed long long   int64_t; */
/* typedef unsigned long long uint64_t; */

#define INT8_MIN (-128)
#define INT16_MIN (-32768)
#define INT32_MIN (-2147483647 - 1)

#define INT8_MAX 127
#define INT16_MAX 32767
#define INT32_MAX 2147483647

#define UINT8_MAX 0xff  /* 255U */
#define UINT16_MAX 0xffff       /* 65535U */
#define UINT32_MAX 0xffffffff   /* 4294967295U */

#endif /* STDINT_H */
