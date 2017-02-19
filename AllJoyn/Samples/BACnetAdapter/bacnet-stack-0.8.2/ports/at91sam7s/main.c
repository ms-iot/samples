/**************************************************************************
*
* Copyright (C) 2007 Steve Karg <skarg@users.sourceforge.net>
* Portions of the AT91SAM7S startup code were developed by James P Lynch.
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
/* hardware specific */
#include "board.h"
#include "timer.h"
/* standard libraries */
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
/* BACnet */
#include "rs485.h"
#include "datalink.h"
#include "npdu.h"
#include "apdu.h"
#include "dcc.h"
#include "iam.h"
#include "handlers.h"
#include "client.h"
#include "device.h"
#include "dcc.h"
#include "iam.h"
#include "txbuf.h"

/*  ******************************************************* */
/*   FIXME: use header files?     External References */
/*  ******************************************************* */
extern void LowLevelInit(
    void);
extern unsigned enableIRQ(
    void);
extern unsigned enableFIQ(
    void);

/* used by crt.s file */
unsigned FiqCount = 0;

static unsigned long LED_Timer_1 = 0;
static unsigned long LED_Timer_2 = 0;
static unsigned long LED_Timer_3 = 0;
static unsigned long LED_Timer_4 = 1000;
static unsigned long DCC_Timer = 1000;

static inline void millisecond_timer(
    void)
{
    while (Timer_Milliseconds) {
        Timer_Milliseconds--;
        if (LED_Timer_1) {
            LED_Timer_1--;
        }
        if (LED_Timer_2) {
            LED_Timer_2--;
        }
        if (LED_Timer_3) {
            LED_Timer_3--;
        }
        if (LED_Timer_4) {
            LED_Timer_4--;
        }
        if (DCC_Timer) {
            DCC_Timer--;
        }
    }
    /* note: MS/TP silence timer is updated in ISR */
}

static inline void init(
    void)
{
    unsigned int pcsr;

    /* Initialize the Parallel I/O Controller A Peripheral Clock */
    volatile AT91PS_PMC pPMC = AT91C_BASE_PMC;

    pcsr = pPMC->PMC_PCSR;
    pPMC->PMC_PCER = pcsr | (1 << AT91C_ID_PIOA);

    /* Set up the LEDs (PA0 - PA3) */
    volatile AT91PS_PIO pPIO = AT91C_BASE_PIOA;
    /* PIO Enable Register */
    /* allow PIO to control pins P0 - P3 and pin 19 */
    pPIO->PIO_PER = LED_MASK | SW1_MASK;
    /* PIO Output Enable Register */
    /* sets pins P0 - P3 to outputs */
    pPIO->PIO_OER = LED_MASK;
    /* PIO Set Output Data Register */
    /* turns off the four LEDs */
    pPIO->PIO_SODR = LED_MASK;

    /* Select PA19 (pushbutton) to be FIQ function (Peripheral B) */
    pPIO->PIO_BSR = SW1_MASK;

    /* Set up the AIC registers for FIQ (pushbutton SW1) */
    volatile AT91PS_AIC pAIC = AT91C_BASE_AIC;
    /* Disable FIQ interrupt in */
    /* AIC Interrupt Disable Command Register */
    pAIC->AIC_IDCR = (1 << AT91C_ID_FIQ);
    /* Set the interrupt source type in */
    /* AIC Source Mode Register[0] */
    pAIC->AIC_SMR[AT91C_ID_FIQ] = (AT91C_AIC_SRCTYPE_INT_EDGE_TRIGGERED);
    /* Clear the FIQ interrupt in */
    /* AIC Interrupt Clear Command Register */
    pAIC->AIC_ICCR = (1 << AT91C_ID_FIQ);
    /* Remove disable FIQ interrupt in */
    /* AIC Interrupt Disable Command Register */
    pAIC->AIC_IDCR = (0 << AT91C_ID_FIQ);
    /* Enable the FIQ interrupt in */
    /* AIC Interrupt Enable Command Register */
    pAIC->AIC_IECR = (1 << AT91C_ID_FIQ);
}

static inline void bacnet_init(
    void)
{
#if defined(BACDL_MSTP)
    uint8_t MAC_Address = 0x55;

    RS485_Set_Baud_Rate(38400);
    dlmstp_set_mac_address(MAC_Address);
    dlmstp_set_max_master(127);
    dlmstp_set_max_info_frames(1);
    dlmstp_init(NULL);
#endif
    Device_Set_Object_Instance_Number(22222);
    /* initialize objects */
    Device_Init(NULL);
    /* set up our confirmed service unrecognized service handler - required! */
    apdu_set_unrecognized_service_handler_handler
        (handler_unrecognized_service);
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_HAS, handler_who_has);
    /* we need to handle who-is to support dynamic device binding */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);
    /* Set the handlers for any confirmed services that we support. */
    /* We must implement read property - it's required! */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
        handler_read_property);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROP_MULTIPLE,
        handler_read_property_multiple);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_REINITIALIZE_DEVICE,
        handler_reinitialize_device);
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROPERTY,
        handler_write_property);
    /* handle communication so we can shutup when asked */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL,
        handler_device_communication_control);
}

static uint8_t Receive_PDU[MAX_MPDU];   /* PDU data */
int main(
    void)
{
    unsigned long IdleCount = 0;        /* idle loop blink counter */
    bool LED1_Off_Enabled = true;
    bool LED2_Off_Enabled = true;
    bool LED3_Off_Enabled = true;
    uint16_t pdu_len = 0;
    BACNET_ADDRESS src; /* source address */
    /* Set up the LEDs (PA0 - PA3) */
    volatile AT91PS_PIO pPIO = AT91C_BASE_PIOA;

    /* Initialize the Atmel AT91SAM7S256 */
    /* (watchdog, PLL clock, default interrupts, etc.) */
    LowLevelInit();
    TimerInit();
    init();
    bacnet_init();
    /* enable interrupts */
    isr_enable();
    /* broadcast an I-Am on startup */
    Send_I_Am(&Handler_Transmit_Buffer[0]);
    /* endless blink loop */
    while (1) {
        millisecond_timer();
        if (!DCC_Timer) {
            dcc_timer_seconds(1);
            DCC_Timer = 1000;
        }
        /* USART Tx turns the LED on, we turn it off */
        if (((pPIO->PIO_ODSR & LED1) == LED1) && (LED1_Off_Enabled)) {
            LED1_Off_Enabled = false;
            /* wait */
            LED_Timer_1 = 20;
        }
        if (!LED_Timer_1) {
            /* turn off */
            pPIO->PIO_SODR = LED1;
            LED1_Off_Enabled = true;
        }
        /* USART Rx turns the LED on, we turn it off */
        if (((pPIO->PIO_ODSR & LED2) == LED2) && (LED2_Off_Enabled)) {
            LED2_Off_Enabled = false;
            /* wait */
            LED_Timer_2 = 20;
        }
        if (!LED_Timer_2) {
            /* turn off */
            pPIO->PIO_SODR = LED2;
            LED2_Off_Enabled = true;
        }
        /* switch or NPDU turns on the LED, we turn it off */
        if (((pPIO->PIO_ODSR & LED3) == LED3) && (LED3_Off_Enabled)) {
            LED3_Off_Enabled = false;
            /* wait */
            LED_Timer_3 = 500;
        }
        if (!LED_Timer_3) {
            /* turn LED3 (DS3) off */
            pPIO->PIO_SODR = LED3;
            LED3_Off_Enabled = true;
        }
        /* Blink LED every second */
        if (!LED_Timer_4) {
            if ((pPIO->PIO_ODSR & LED4) == LED4) {
                /* turn on */
                pPIO->PIO_CODR = LED4;
            } else {
                /* turn off */
                pPIO->PIO_SODR = LED4;
            }
            /* wait */
            LED_Timer_4 = 1000;
        }
        /* count # of times through the idle loop */
        IdleCount++;
        /* BACnet handling */
        pdu_len =
            datalink_receive(&src, &Receive_PDU[0], sizeof(Receive_PDU), 0);
        if (pdu_len) {
            pPIO->PIO_CODR = LED3;
            npdu_handler(&src, &Receive_PDU[0], pdu_len);
        }
    }
}
