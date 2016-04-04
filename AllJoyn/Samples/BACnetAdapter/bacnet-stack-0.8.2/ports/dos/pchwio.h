/*
**  PCHWIO.H - SNIPPETS header file for portable hardware I/O access under DOS
**
**  public domain by Bob Stout
*/

#ifndef PCHWIO__H
#define PCHWIO__H

#include <dos.h>
#include "extkword.h"


#if defined(__TURBOC__) || defined(__POWERC)
#ifndef inp
#define inp           inportb
#endif
#ifndef outp
#define outp          outportb
#endif
#ifndef inpw
#define inpw          inport
#endif
#ifndef outpw
#define outpw         outport
#endif
#elif defined(__ZTC__)
#include <int.h>
#define enable         int_on
#define disable        int_off
#if !defined(__SC__)
void FAR *getvect(
    unsigned intnum);
void setvect(
    unsigned intnum,
    void (INTERRUPT FAR * handler) ());
#else
#define getvect       _dos_getvect
#define setvect       _dos_setvect
#endif
#else /* assume MSC/QC/WC */
#include <conio.h>
#if defined(__WATCOMC__)
#include <i86.h>
#endif
#define enable         _enable
#define disable        _disable
#define getvect        _dos_getvect
#define setvect        _dos_setvect
#endif


#if defined(_MSC_VER) || defined(__WATCOMC__) || \
      defined(__ZTC__) || defined(__SC__)

unsigned char Peekb(
    unsigned seg,
    unsigned ofs);      /* PCHWIO.C */
unsigned short Peekw(
    unsigned seg,
    unsigned ofs);      /* PCHWIO.C */
void Pokeb(
    unsigned seg,
    unsigned ofs,
    unsigned char ch);  /* PCHWIO.C */
void Pokew(
    unsigned seg,
    unsigned ofs,
    unsigned short num);        /* PCHWIO.C */

#elif defined(__TURBOC__)
#define Peekw peek
#define Pokew poke
#define Peekb peekb
#define Pokeb pokeb
#endif /* peek(), poke() */

#endif /* PCHWIO__H */
