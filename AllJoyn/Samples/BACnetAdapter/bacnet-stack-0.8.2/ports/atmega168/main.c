/**************************************************************************
*
* Copyright (C) 2007 Steve Karg <skarg@users.sourceforge.net>
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

#include <stdbool.h>
#include <stdint.h>
#include "hardware.h"
#include "timer.h"
#include "rs485.h"
#include "datalink.h"
#include "npdu.h"
#include "handlers.h"
#include "txbuf.h"
#include "iam.h"
#include "device.h"
#include "av.h"

/* From the WhoIs hander - performed by the DLMSTP module */
extern bool Send_I_Am_Flag;
/* local version override */
const char *BACnet_Version = "1.0";

/* For porting to IAR, see:
   http://www.avrfreaks.net/wiki/index.php/Documentation:AVR_GCC/IarToAvrgcc*/

/* dummy function - so we can use default demo handlers */
bool dcc_communication_enabled(
    void)
{
    return true;
}

static void init(
    void)
{
    /* Initialize the Clock Prescaler for ATmega48/88/168 */
    /* The default CLKPSx bits are factory set to 0011 */
    /* Enbable the Clock Prescaler */
    CLKPR = _BV(CLKPCE);
    /* CLKPS3 CLKPS2 CLKPS1 CLKPS0 Clock Division Factor
       ------ ------ ------ ------ ---------------------
       0      0      0      0             1
       0      0      0      1             2
       0      0      1      0             4
       0      0      1      1             8
       0      1      0      0            16
       0      1      0      1            32
       0      1      1      0            64
       0      1      1      1           128
       1      0      0      0           256
       1      x      x      x      Reserved
     */
    /* Set the CLKPS3..0 bits to Prescaler of 1 */
    CLKPR = 0;
    /* Initialize I/O ports */
    /* For Port DDRx (Data Direction) Input=0, Output=1 */
    /* For Port PORTx (Bit Value) TriState=0, High=1 */
    DDRB = 0;
    PORTB = 0;
    DDRC = 0;
    PORTC = 0;
    DDRD = 0;
    PORTD = 0;

    /* Configure the watchdog timer - Disabled for testing */
    BIT_CLEAR(MCUSR, WDRF);
    WDTCSR = 0;

    /* Configure Specialized Hardware */
    RS485_Initialize();

    /* configure one LED for NPDU indication */
    /* default: off, output */
    LED_NPDU_OFF();
    LED_NPDU_INIT();
    /* Configure Software LED */
    LED_GREEN_INIT();
    LED_GREEN_OFF();

    /* Configure Timer0 for millisecond timer */
    Timer_Initialize();

    /* Enable global interrupts */
    __enable_interrupt();
}

static void task_milliseconds(
    void)
{
    while (Timer_Milliseconds) {
        Timer_Milliseconds--;
        /* add other millisecond timer tasks here */
        RS485_LED_Timers();
    }
}

static uint8_t Address_Switch;

static void input_switch_read(
    void)
{
    uint8_t value;
    static uint8_t old_value = 0;

    value = BITMASK_CHECK(PINC, 0x0F);
    value |= (BITMASK_CHECK(PINB, 0x07) << 4);
    if (value != old_value) {
        old_value = value;
    } else {
        if (old_value != Address_Switch) {
            Address_Switch = old_value;
#if defined(BACDL_MSTP)
            dlmstp_set_mac_address(Address_Switch);
#endif
            Device_Set_Object_Instance_Number(86000 + Address_Switch);
            Send_I_Am_Flag = true;
        }
    }
}

static uint8_t PDUBuffer[MAX_MPDU];
int main(
    void)
{
    uint16_t pdu_len = 0;
    BACNET_ADDRESS src; /* source address */

    init();
#if defined(BACDL_MSTP)
    RS485_Set_Baud_Rate(38400);
    dlmstp_set_max_master(127);
    dlmstp_set_max_info_frames(1);
#endif
    datalink_init(NULL);
    for (;;) {
        input_switch_read();
        task_milliseconds();
        /* other tasks */
        /* BACnet handling */
        pdu_len = datalink_receive(&src, &PDUBuffer[0], sizeof(PDUBuffer), 0);
        if (pdu_len) {
            LED_NPDU_ON();
            npdu_handler(&src, &PDUBuffer[0], pdu_len);
            LED_NPDU_OFF();
        }
    }
}
