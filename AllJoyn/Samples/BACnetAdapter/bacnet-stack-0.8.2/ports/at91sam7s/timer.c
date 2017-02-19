/* ***************************************************************************** */
/* */
/*     Purpose:  Set up the 16-bit Timer/Counter */
/* */
/*     We will use Timer Channel 0 to develop a 1 msec interrupt. */
/* */
/*     The AT91SAM7S-EK board has a 18,432,000 hz crystal oscillator. */
/* */
/*     MAINCK  = 18432000 hz */
/*     PLLCK = (MAINCK / DIV) * (MUL + 1) = 18432000/14 * (72 + 1) */
/*     PLLCLK = 1316571 * 73 = 96109683 hz */
/* */
/*       MCK = PLLCLK / 2 = 96109683 / 2 = 48054841 hz */
/* */
/*     TIMER_CLOCK5 = MCK / 1024  = 48054841 / 1024  =  46928 hz */
/* */
/*     TIMER_CLOCK5 Period = 1 / 46928  =  21.309239686 microseconds */
/* */
/*     A little algebra:  .001 sec = count * 21.3092396896*10**-6 */
/*                        count =  .001 / 21.3092396896*10**-6 */
/*                        count =  46.928 */
/* */
/* */
/*     Therefore:  set Timer Channel 0 register RC to 46*milliseconds */
/*                 turn on capture mode WAVE = 0 */
/*                 enable the clock  CLKEN = 1 */
/*                 select TIMER_CLOCK5  TCCLKS = 100 */
/*                 clock is NOT inverted CLKI = 0 */
/*                 enable RC compare CPCTRG = 1 */
/*                 enable RC compare interrupt  CPCS = 1 */
/*                 disable all the other timer 0 interrupts */
/* */
/* Author:  James P Lynch  May 12, 2007 */
/* Modified by Steve Karg */
/*     Changed timer to 1ms. */
/*     Encapsulated the intialization */
/* ***************************************************************************** */


/**********************************************************
                    Header files
 **********************************************************/
#include <stdint.h>
#include "board.h"
#include "dlmstp.h"

/* global variable counts interrupts */
volatile unsigned long Timer_Milliseconds;
/* MS/TP Silence Timer */
static volatile int SilenceTime;

static void Timer0_Setup(
    int milliseconds)
{
    /*        TC Block Control Register TC_BCR    (read/write) */
    /* */
    /*    |------------------------------------------------------------------|------| */
    /*    |                                                                   SYNC  | */
    /*    |------------------------------------------------------------------|------| */
    /*   31                                                               1    0 */
    /* */
    /*    SYNC = 0 (no effect)    <===== take  default */
    /*    SYNC = 1 (generate software trigger for all 3 timer channels simultaneously) */
    /* */
    /* create a pointer to TC Global Register structure */
    AT91PS_TCB pTCB = AT91C_BASE_TCB;
    /* SYNC trigger not used */
    pTCB->TCB_BCR = 0;

    /*        TC Block Mode Register  TC_BMR    (read/write) */
    /* */
    /*    |-------------------------------------|-----------|-----------|-----------| */
    /*    |                                       TC2XC2S     TCXC1S       TC0XC0S  | */
    /*    |-------------------------------------|-----------|-----------|-----------| */
    /*   31                                     5       4   3       2   1         0 */
    /* */
    /*    TC0XC0S Select = 00 TCLK0 (PA4) */
    /*                   = 01 none           <===== we select this one */
    /*                 = 10 TIOA1 (PA15) */
    /*                 = 11 TIOA2 (PA26) */
    /* */
    /*    TCXC1S  Select = 00 TCLK1 (PA28) */
    /*                   = 01 none           <===== we select this one */
    /*                 = 10 TIOA0 (PA15) */
    /*                 = 11 TIOA2 (PA26) */
    /* */
    /*    TC2XC2S Select = 00 TCLK2 (PA29) */
    /*                   = 01 none           <===== we select this one */
    /*                 = 10 TIOA0 (PA00) */
    /*                 = 11 TIOA1 (PA26) */
    /* */
    /* external clocks not used */
    pTCB->TCB_BMR = 0x15;


    /*        TC Channel Control Register  TC_CCR    (read/write) */
    /* */
    /*    |----------------------------------|--------------|------------|-----------| */
    /*    |                                      SWTRG          CLKDIS       CLKENS  | */
    /*    |----------------------------------|--------------|------------|-----------| */
    /*   31                                        2             1           0 */
    /* */
    /*    CLKEN = 0    no effect */
    /*    CLKEN = 1    enables the clock     <===== we select this one */
    /* */
    /*    CLKDIS = 0   no effect             <===== take  default */
    /*    CLKDIS = 1   disables the clock */
    /* */
    /*  SWTRG = 0    no effect */
    /*  SWTRG = 1    software trigger aserted counter reset and clock starts   <===== we select this one */
    /* */
    /* create a pointer to channel 0 Register structure */
    AT91PS_TC pTC = AT91C_BASE_TC0;
    /* enable the clock    and start it */
    pTC->TC_CCR = 0x5;

    /*        TC Channel Mode Register  TC_CMR    (read/write) */
    /* */
    /*    |-----------------------------------|------------|---------------| */
    /*    |                                      LDRB            LDRA      | */
    /*    |-----------------------------------|------------|---------------| */
    /*   31                                  19        18 17           16 */
    /* */
    /*    |----------|---------|--------------|------------|---------------| */
    /*    |WAVE = 0    CPCTRG                    ABETRG         ETRGEDG    | */
    /*    |----------|---------|--------------|------------|---------------| */
    /*      15         14     13          11      10      9             8 */
    /* */
    /*    |----------|---------|--------------|------------|---------------| */
    /*    |  LDBDIS    LDBSTOP     BURST         CLKI            TCCLKS    | */
    /*    |----------|---------|--------------|------------|---------------| */
    /*       7          6     5            4      3       2             0 */
    /* */
    /*  CLOCK SELECTION */
    /*    TCCLKS = 000  TIMER_CLOCK1    (MCK/2    =  24027420 hz) */
    /*             001  TIMER_CLOCK2    (MCK/8    =   6006855 hz) */
    /*             010  TIMER_CLOCK3    (MCK/32   =   1501713 hz) */
    /*             011  TIMER_CLOCK4    (MCK/128  =    375428 hz) */
    /*             100  TIMER_CLOCK5    (MCK/1024 =     46928 hz)   <===== we select this one */
    /*           101  XC0 */
    /*           101  XC1 */
    /*           101  XC2 */
    /* */
    /*  CLOCK INVERT */
    /*    CLKI   = 0   counter incremented on rising clock edge    <===== we select this one */
    /*    CLKI   = 1   counter incremented on falling clock edge */
    /* */
    /*  BURST SIGNAL SELECTION */
    /*  BURST  = 00  clock is not gated by any external system   <===== take  default */
    /*           01  XC0 is anded with the clock */
    /*           10  XC1 is anded with the clock */
    /*           11  XC2 is anded with the clock */
    /* */
    /*  COUNTER CLOCK STOPPED WITH RB LOADING */
    /*  LDBSTOP  = 0  counter clock is not stopped when RB loading occurs   <===== take  default */
    /*           = 1  counter clock is stopped when RB loading occur */
    /* */
    /*  COUNTER CLOCK DISABLE WITH RB LOADING */
    /*  LDBDIS   = 0  counter clock is not disabled when RB loading occurs   <===== take  default */
    /*           = 1  counter clock is disabled when RB loading occurs */
    /* */
    /*  EXTERNAL TRIGGER EDGE SELECTION */
    /*  ETRGEDG  = 00  (none)              <===== take default */
    /*             01  (rising edge) */
    /*             10  (falling edge) */
    /*             11  (each edge) */
    /* */
    /*  TIOA OR TIOB EXTERNAL TRIGGER SELECTION */
    /*  ABETRG   = 0  (TIOA is used)      <===== take default */
    /*             1  (TIOB is used) */
    /* */
    /*  RC COMPARE TRIGGER ENABLE */
    /*  CPCTRG   = 0  (RC Compare has no effect on the counter and its clock) */
    /*             1  (RC Compare resets the counter and starts the clock)      <===== we select this one */
    /* */
    /*  WAVE */
    /*  WAVE     = 0  Capture Mode is enabled     <===== we select this one */
    /*             1  Waveform Mode is enabled */
    /* */
    /*  RA LOADING SELECTION */
    /*  LDRA  = 00 none)                   <===== take default */
    /*          01 (rising edge of TIOA) */
    /*          10 (falling edge of TIOA) */
    /*          11 (each edge of TIOA) */
    /* */
    /*  RB LOADING SELECTION */
    /*  LDRB  = 00 (none)                  <===== take default */
    /*          01 (rising edge of TIOA) */
    /*          10 (falling edge of TIOA) */
    /*          11 (each edge of TIOA) */
    /* */
    /* TCCLKS = 1 (TIMER_CLOCK5) */
    /* CPCTRG = 1 (RC Compare resets the counter and restarts the clock) */
    /* WAVE   = 0 (Capture mode enabled) */
    pTC->TC_CMR = 0x4004;

    /*        TC Register C     TC_RC   (read/write)   Compare Register 16-bits */
    /* */
    /*    |----------------------------------|----------------------------------------| */
    /*    |           not used                               RC                       | */
    /*    |----------------------------------|----------------------------------------| */
    /*   31                              16 15                                    0 */
    /* */
    /*    Timer Calculation:   What count gives 1 msec time-out? */
    /* */
    /*    TIMER_CLOCK5 = MCK / 1024  = 48054841 / 1024  =  46928 hz */
    /* */
    /*    TIMER_CLOCK5 Period = 1 / 46928  =  21.309239686 microseconds */
    /* */
    /*    A little algebra:  .001 sec = count * 21.3092396896*10**-6 */
    /*                        count =  .001 / 21.3092396896*10**-6 */
    /*                        count =  46.928 */
    /* */
    /*    STK: Even Simpler, let the compiler do the work: */
    /* */
    /*    TIMER_CLOCK5 = (MCK / 1024) / 1000 */
    /*                 = 48054841 / 1024 / 1000 =  46.928 */
    pTC->TC_RC = ((MCK / 1024 / 1000) + 1) * milliseconds;

    /*        TC Interrupt Enable Register  TC_IER    (write-only) */
    /* */
    /* */
    /*    |------------|-------|-------|-------|-------|--------|--------|--------|--------| */
    /*    |              ETRGS   LDRBS   LDRAS   CPCS    CPBS      CPAS     LOVRS    COVFS | */
    /*    |------------|-------|-------|-------|-------|--------|--------|--------|--------| */
    /*   31         8    7       6       5       4        3        2        1        0 */
    /* */
    /*  COVFS    = 0  no effect    <===== take  default */
    /*             1  enable counter overflow interrupt */
    /* */
    /*  LOVRS    = 0  no effect    <===== take  default */
    /*             1  enable load overrun interrupt */
    /* */
    /*  CPAS     = 0  no effect    <===== take  default */
    /*             1  enable RA compare interrupt */
    /* */
    /*  CPBS     = 0  no effect    <===== take  default */
    /*             1  enable RB compare interrupt */
    /* */
    /*  CPCS     = 0  no effect */
    /*             1  enable RC compare interrupt    <===== we select this one */
    /* */
    /*  LDRAS    = 0  no effect    <===== take  default */
    /*             1  enable RA load interrupt */
    /* */
    /*  LDRBS    = 0  no effect    <===== take  default */
    /*             1  enable RB load interrupt */
    /* */
    /*  ETRGS    = 0  no effect    <===== take  default */
    /*             1  enable External Trigger interrupt */
    /* */
    /* enable RC compare interrupt */
    pTC->TC_IER = 0x10;

    /*        TC Interrupt Disable Register  TC_IDR    (write-only) */
    /* */
    /* */
    /*    |------------|-------|-------|-------|-------|--------|--------|--------|--------| */
    /*    |              ETRGS   LDRBS   LDRAS   CPCS    CPBS      CPAS     LOVRS    COVFS | */
    /*    |------------|-------|-------|-------|-------|--------|--------|--------|--------| */
    /*   31         8    7       6       5       4        3        2        1        0 */
    /* */
    /*  COVFS    = 0  no effect */
    /*             1  disable counter overflow interrupt    <===== we select this one */
    /* */
    /*  LOVRS    = 0  no effect */
    /*             1  disable load overrun interrupt    <===== we select this one */
    /* */
    /*  CPAS     = 0  no effect */
    /*             1  disable RA compare interrupt    <===== we select this one */
    /* */
    /*  CPBS     = 0  no effect */
    /*             1  disable RB compare interrupt    <===== we select this one */
    /* */
    /*  CPCS     = 0  no effect    <===== take  default */
    /*             1  disable RC compare interrupt */
    /* */
    /*  LDRAS    = 0  no effect */
    /*             1  disable RA load interrupt    <===== we select this one */
    /* */
    /*  LDRBS    = 0  no effect */
    /*             1  disable RB load interrupt    <===== we select this one */
    /* */
    /*  ETRGS    = 0  no effect */
    /*             1  disable External Trigger interrupt    <===== we select this one */
    /* */
    /* disable all except RC compare interrupt */
    pTC->TC_IDR = 0xEF;
}

/*  ***************************************************************************** */
/* */
/*     Timer 0 Interrupt Service Routine */
/* */
/*     Entered when Timer0 RC compare interrupt asserts */
/* */
/*  Author:  James P Lynch  May 12, 2007 */
/*  Modified by Steve Karg */
/*    simplified and changed to a millisecond count-up timer */
/*  ***************************************************************************** */
static void Timer0IrqHandler(
    void)
{

    volatile AT91PS_TC pTC = AT91C_BASE_TC0;    /* pointer to timer channel 0 register structure */
    volatile unsigned int dummy;        /* temporary */

    /* read TC0 Status Register to clear interrupt */
    dummy = pTC->TC_SR;
    /* increment the tick count */
    Timer_Milliseconds++;
    if (SilenceTime < 60000)
        SilenceTime++;
}

int Timer_Silence(
    void)
{
    return SilenceTime;
}

void Timer_Silence_Reset(
    void)
{
    SilenceTime = 0;
}

/*  ***************************************************************************** */
/* */
/*     Timer 0 Initialization */
/* */
/*  From James P Lynch main.c example code */
/*  Modified by Steve Karg */
/*    Moved timer startup code from main */
/*    modified the peripheral clock init */
/*  ***************************************************************************** */
void TimerInit(
    void)
{
    unsigned int pcsr;
    /* enable the Timer0 peripheral clock */
    volatile AT91PS_PMC pPMC = AT91C_BASE_PMC;

    pcsr = pPMC->PMC_PCSR;
    pPMC->PMC_PCER = pcsr | (1 << AT91C_ID_TC0);
    /* Set up the AIC  registers for Timer 0 */
    volatile AT91PS_AIC pAIC = AT91C_BASE_AIC;
    /* Disable timer 0 interrupt */
    /* in AIC Interrupt Disable Command Register */
    pAIC->AIC_IDCR = (1 << AT91C_ID_TC0);
    /* Set the TC0 IRQ handler address in */
    /* AIC Source Vector Register[12] */
    pAIC->AIC_SVR[AT91C_ID_TC0] = (unsigned int) Timer0IrqHandler;
    /* Set the interrupt source type and priority */
    /* in AIC Source Mode Register[12] */
    pAIC->AIC_SMR[AT91C_ID_TC0] =
        (AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE | 0x4);
    /* Clear the TC0 interrupt */
    /* in AIC Interrupt Clear Command Register */
    pAIC->AIC_ICCR = (1 << AT91C_ID_TC0);
    /* Remove disable timer 0 interrupt */
    /* in AIC Interrupt Disable Command Reg */
    pAIC->AIC_IDCR = (0 << AT91C_ID_TC0);
    /* Enable the TC0 interrupt */
    /* in AIC Interrupt Enable Command Register */
    pAIC->AIC_IECR = (1 << AT91C_ID_TC0);
    /* Setup timer0 to generate a 1 msec periodic interrupt */
    Timer0_Setup(1);
}
