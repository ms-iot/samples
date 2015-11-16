/*
**  PCHWIO.C - SNIPPETS portable hardware I/O access under DOS
**
**  public domain by Bob Stout
*/

#include "pchwio.h"
#include "mk_fp.h"

#if defined(__ZTC__) && !defined(__SC__)

void FAR *getvect(
    unsigned intnum)
{
    unsigned seg, off;

    int_getvector(intnum, &off, &seg);
    return MK_FP(seg, off);
}

void setvect(
    unsigned intnum,
    void (INTERRUPT FAR * handler) ())
{
    unsigned seg = FP_SEG(handler), off = FP_OFF(handler);

    int_setvector(intnum, off, seg);
}

#endif /* ZTC getvect(), setvect() */



#if defined(_MSC_VER) || defined(__WATCOMC__) || \
      defined(__ZTC__) || defined(__SC__)

#if !defined(MK_FP)
#define MK_FP(seg,off) ((void far *)(((long)(seg) << 16)|(unsigned)(off)))
#endif

unsigned char Peekb(
    unsigned seg,
    unsigned ofs)
{
    unsigned char FAR *ptr;

    ptr = MK_FP(seg, ofs);
    return *ptr;
}

unsigned short Peekw(
    unsigned seg,
    unsigned ofs)
{
    unsigned FAR *ptr;

    ptr = MK_FP(seg, ofs);
    return *ptr;
}

void Pokeb(
    unsigned seg,
    unsigned ofs,
    unsigned char ch)
{
    unsigned char FAR *ptr;

    ptr = MK_FP(seg, ofs);
    *ptr = ch;
}

void Pokew(
    unsigned seg,
    unsigned ofs,
    unsigned short num)
{
    unsigned FAR *ptr;

    ptr = MK_FP(seg, ofs);
    *ptr = num;
}

#endif /* MSC/ZTC/WC Peek(), poke() */
