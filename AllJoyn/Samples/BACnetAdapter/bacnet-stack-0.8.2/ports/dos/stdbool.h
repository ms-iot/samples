#ifndef _STDBOOL_H
#define _STDBOOL_H

/* C99 Boolean types for compilers without C99 support */
/* http://www.opengroup.org/onlinepubs/009695399/basedefs/stdbool.h.html */
#if !defined(__cplusplus)

#if !defined(__GNUC__)
/* _Bool builtin type is included in GCC */
typedef enum { _Bool_must_promote_to_int = -1, false = 0, true = 1 } _Bool;
#endif

#define bool _Bool
#define true 1
#define false 0
#define __bool_true_false_are_defined 1

#endif

#endif
