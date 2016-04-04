#ifndef STDBOOL_H
#define STDBOOL_H

/* C99 Boolean types for compilers without C99 support */

#ifndef __cplusplus

/*typedef int _Bool; */
#ifndef bool
#define bool _Bool
#endif
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
#define __bool_true_false_are_defined 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#endif
