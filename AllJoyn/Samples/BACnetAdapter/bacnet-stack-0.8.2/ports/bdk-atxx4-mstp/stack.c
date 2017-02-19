/**************************************************************************
*
* Copyright (C) 2009 Steve Karg <skarg@users.sourceforge.net>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/
#include "hardware.h"
/* me */
#include "stack.h"

#if defined(__GNUC__)
/* stack checking */
extern uint8_t _end;
extern uint8_t __stack;

#define STACK_CANARY (0xC5)
void stack_init(
    void) __attribute__ ((naked)) __attribute__ ((section(".init1")));

void stack_init(
    void)
{
#if 0
    uint8_t *p = &_end;

    while (p <= &__stack) {
        *p = STACK_CANARY;
        p++;
    }
#else
    __asm volatile (
        "    ldi r30,lo8(_end)\n" "    ldi r31,hi8(_end)\n" "    ldi r24,lo8(0xc5)\n"   /* STACK_CANARY = 0xc5 */
        "    ldi r25,hi8(__stack)\n" "    rjmp .cmp\n" ".loop:\n"
        "    st Z+,r24\n" ".cmp:\n" "    cpi r30,lo8(__stack)\n"
        "    cpc r31,r25\n" "    brlo .loop\n" "    breq .loop"::);
#endif
}

unsigned stack_size(
    void)
{
    return (&__stack) - (&_end);
}

uint8_t stack_byte(
    unsigned offset)
{
    return *(&_end + offset);
}

unsigned stack_unused(
    void)
{
    uint8_t *p = &_end;
    unsigned count = 0;

    while (p <= &__stack) {
        if ((*p) != STACK_CANARY) {
            count = p - (&_end);
            break;
        }
        p++;
    }
    return count;
}
#else
void stack_init(
    void)
{

}

unsigned stack_size(
    void)
{
    return 0;
}

uint8_t stack_byte(
    unsigned offset)
{
    return 0;
}

unsigned stack_unused(
    void)
{
    return 0;
}
#endif
