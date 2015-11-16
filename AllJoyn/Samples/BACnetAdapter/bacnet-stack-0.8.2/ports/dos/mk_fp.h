/*
**  MK_FP.H
**
**  Standard header file making sure this pesky Intel macro is defined!
*/

#ifndef MK_FP__H
#define MK_FP__H

#include "extkword.h"

#if defined(__WATCOMC__)
#include <i86.h>
#elif !defined(__PACIFIC__)
#include <dos.h>
#endif

#if !defined(MK_FP)
#define MK_FP(seg,off) \
      ((void FAR *)(((unsigned long)(seg) << 16)|(unsigned)(off)))
#endif

#endif /* MK_FP__H */
