#ifndef _STDBOOL_H
#define _STDBOOL_H

#include <stdint.h>

/* C99 Boolean types for compilers without C99 support */
/* http://www.opengroup.org/onlinepubs/009695399/basedefs/stdbool.h.html */
#if !defined(__cplusplus)

#if !defined(__GNUC__)
/* _Bool builtin type is included in GCC */
/* ISO C Standard: 5.2.5 An object declared as 
   type _Bool is large enough to store 
   the values 0 and 1. */
/* We choose 8 bit to match C++ */
/* It must also promote to integer */
#if _MSC_VER < 1600
typedef int8_t _Bool;
#endif /* _MSC_VER < 1600 VS 2010 and earlier */
#endif

/* ISO C Standard: 7.16 Boolean type */
#define bool _Bool
#define true 1
#define false 0
#define __bool_true_false_are_defined 1

#endif

#endif
