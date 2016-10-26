/*  ---------------------------------------------------------------------------- */
/*           ATMEL Microcontroller Software Support  -  ROUSSET  - */
/*  ---------------------------------------------------------------------------- */
/*   The software is delivered "AS IS" without warranty or condition of any */
/*   kind, either express, implied or statutory. This includes without */
/*   limitation any warranty or condition with respect to merchantability or */
/*   fitness for any particular purpose, or against the infringements of */
/*   intellectual property rights of others. */
/*  ---------------------------------------------------------------------------- */
/*   File Name           : Cstartup_SAM7.c */
/*   Object              : Low level initializations written in C for IAR tools */
/*   1.0   08/Sep/04 JPP : Creation */
/*   1.10  10/Sep/04 JPP : Update AT91C_CKGR_PLLCOUNT filed */
/*  ---------------------------------------------------------------------------- */


/* Include the board file description */
#include "board.h"

/* The following functions must be write in ARM mode this function called directly */
/* by exception vector */
extern void AT91F_Spurious_handler(
    void);
extern void AT91F_Default_IRQ_handler(
    void);
extern void AT91F_Default_FIQ_handler(
    void);

/**---------------------------------------------------------------------------- */
/** \fn    AT91F_LowLevelInit */
/** \brief This function performs very low level HW initialization */
/**        this function can be use a Stack, depending the compilation */
/**        optimization mode */
/**---------------------------------------------------------------------------- */
void LowLevelInit(
    void)
{
    int i;
    AT91PS_PMC pPMC = AT91C_BASE_PMC;

    /** Set Flash Wait sate */
    /*  Single Cycle Access at Up to 30 MHz, or 40 */
    /*  if MCK = 48054841 I have 50 Cycle for 1 usecond ( flied MC_FMR->FMCN */
    /*  result: AT91C_MC_FMR = 0x00320100  (MC Flash Mode Register) */
    AT91C_BASE_MC->MC_FMR =
        (((AT91C_MC_FMCN) & (50 << 16)) | AT91C_MC_FWS_1FWS);

    /** Watchdog Disable */
    /* result: AT91C_WDTC_WDMR = 0x00008000  (Watchdog Mode Register) */
    AT91C_BASE_WDTC->WDTC_WDMR = AT91C_WDTC_WDDIS;

    /** Set MCK at 48 054 841 */
    /* 1 Enabling the Main Oscillator: */
    /* SCK = 1/32768 = 30.51 uSecond */
    /* Start up time = 8 * 6 / SCK = 56 * 30.51 = 1,46484375 ms */
    /* result: AT91C_CKGR_MOR = 0x00000601  (Main Oscillator Register)  */
    pPMC->PMC_MOR = ((AT91C_CKGR_OSCOUNT & (0x06 << 8)) | AT91C_CKGR_MOSCEN);

    /* Wait the startup time */
    while (!(pPMC->PMC_SR & AT91C_PMC_MOSCS));

    /* PMC Clock Generator PLL Register setup */
    /* */
    /* The following settings are used:  DIV = 14 */
    /*                                   MUL = 72 */
    /*                                   PLLCOUNT = 10 */
    /* */
    /* Main Clock (MAINCK from crystal oscillator) = 18432000 hz (see AT91SAM7-EK schematic) */
    /* MAINCK / DIV = 18432000/14 = 1316571 hz */
    /* PLLCK = 1316571 * (MUL + 1) = 1316571 * (72 + 1) = 1316571 * 73 = 96109683 hz */
    /* */
    /* PLLCOUNT = number of slow clock cycles before the LOCK bit is set  */
    /*            in PMC_SR after CKGR_PLLR is written. */
    /* */
    /* PLLCOUNT = 10 */
    /* */
    /* OUT = 0 (not used) */
    /* result: AT91C_CKGR_PLLR = 0x00000000480A0E   (PLL Register)  */
    pPMC->PMC_PLLR =
        ((AT91C_CKGR_DIV & 14) | (AT91C_CKGR_PLLCOUNT & (10 << 8)) |
        (AT91C_CKGR_MUL & (72 << 16)));

    /* Wait the startup time (until PMC Status register LOCK bit is set) */
    while (!(pPMC->PMC_SR & AT91C_PMC_LOCK));

    /* PMC Master Clock (MCK) Register setup */
    /* */
    /* CSS  = 3  (PLLCK clock selected) */
    /* */
    /* PRES = 1  (MCK = PLLCK / 2) = 96109683/2 = 48054841 hz */
    /* */
    /* Note: Master Clock  MCK = 48054841 hz  (this is the CPU clock speed) */
    /* result:  AT91C_PMC_MCKR = 0x00000007  (Master Clock Register) */
    pPMC->PMC_MCKR = AT91C_PMC_CSS_PLL_CLK | AT91C_PMC_PRES_CLK_2;

    /* Set up the default interrupts handler vectors */
    AT91C_BASE_AIC->AIC_SVR[0] = (int) AT91F_Default_FIQ_handler;
    for (i = 1; i < 31; i++) {
        AT91C_BASE_AIC->AIC_SVR[i] = (int) AT91F_Default_IRQ_handler;
    }
    AT91C_BASE_AIC->AIC_SPU = (int) AT91F_Spurious_handler;
}
