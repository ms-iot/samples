/*  ********************************************************************************************** */
/* */
/*  File Name : isr.c */
/*  Title     : interrupt enable/disable functions */
/* */
/* */
/*  This module provides the interface routines for setting up and */
/*  controlling the various interrupt modes present on the ARM processor. */
/*  Copyright 2004, R O SoftWare */
/*  No guarantees, warrantees, or promises, implied or otherwise. */
/*  May be used for hobby or commercial purposes provided copyright */
/*  notice remains intact. */
/* */
/*  Note from Jim Lynch: */
/*  This module was developed by Bill Knight, RO Software and used with his permission. */
/*  Taken from the Yahoo LPC2000 User's Group - Files Section 'UT050418A.ZIP' */
/*  Specifically, the module armVIC.c with the include file references removed */
/*  ********************************************************************************************** */

#define IRQ_MASK 0x00000080
#define FIQ_MASK 0x00000040
#define INT_MASK (IRQ_MASK | FIQ_MASK)

static inline unsigned __get_cpsr(
    void)
{
    unsigned long retval;
    asm volatile (
        " mrs  %0, cpsr":"=r" (retval): /* no inputs */ );
    return retval;
}

static inline void __set_cpsr(
    unsigned val)
{
    asm volatile (
        " msr  cpsr, %0": /* no outputs */ :"r" (val));
}

unsigned disableIRQ(
    void)
{
    unsigned _cpsr;
    _cpsr = __get_cpsr();
    __set_cpsr(_cpsr | IRQ_MASK);
    return _cpsr;
}

unsigned restoreIRQ(
    unsigned oldCPSR)
{
    unsigned _cpsr;

    _cpsr = __get_cpsr();
    __set_cpsr((_cpsr & ~IRQ_MASK) | (oldCPSR & IRQ_MASK));
    return _cpsr;
}

unsigned enableIRQ(
    void)
{
    unsigned _cpsr;

    _cpsr = __get_cpsr();
    __set_cpsr(_cpsr & ~IRQ_MASK);
    return _cpsr;
}

unsigned disableFIQ(
    void)
{
    unsigned _cpsr;

    _cpsr = __get_cpsr();
    __set_cpsr(_cpsr | FIQ_MASK);
    return _cpsr;
}

unsigned restoreFIQ(
    unsigned oldCPSR)
{
    unsigned _cpsr;

    _cpsr = __get_cpsr();
    __set_cpsr((_cpsr & ~FIQ_MASK) | (oldCPSR & FIQ_MASK));
    return _cpsr;
}

unsigned enableFIQ(
    void)
{
    unsigned _cpsr;

    _cpsr = __get_cpsr();
    __set_cpsr(_cpsr & ~FIQ_MASK);
    return _cpsr;
}
