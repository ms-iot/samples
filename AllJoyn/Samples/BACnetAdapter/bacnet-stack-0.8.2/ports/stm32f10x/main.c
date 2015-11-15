/************************************************************************
*
* Copyright (C) 2011 Steve Karg <skarg@users.sourceforge.net>
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
*************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "hardware.h"
#include "timer.h"
#include "timer.h"
#include "rs485.h"
#include "led.h"
#include "bacnet.h"

/* local version override */
char *BACnet_Version = "1.0";

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(
    uint8_t * file,
    uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1) {
    }
}
#endif

/* Private define ------------------------------------------------------------*/
#define  LSE_FAIL_FLAG  0x80
#define  LSE_PASS_FLAG  0x100

void lse_init(
    void)
{
    uint32_t LSE_Delay = 0;
    struct etimer Delay_Timer;

    /* Enable access to the backup register => LSE can be enabled */
    PWR_BackupAccessCmd(ENABLE);
    /* Enable LSE (Low Speed External Oscillation) */
    RCC_LSEConfig(RCC_LSE_ON);

    /* Check the LSE Status */
    while (1) {
        if (LSE_Delay < LSE_FAIL_FLAG) {
            timer_elapsed_start(&Delay_Timer);
            while (!timer_elapsed_milliseconds(&Delay_Timer, 500)) {
                /* do nothing */
            }
            /* check whether LSE is ready, with 4 seconds timeout */
            LSE_Delay += 0x10;
            if (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET) {
                /* Set flag: LSE PASS */
                LSE_Delay |= LSE_PASS_FLAG;
                led_ld4_off();
                /* Disable LSE */
                RCC_LSEConfig(RCC_LSE_OFF);
                break;
            }
        }

        /* LSE_FAIL_FLAG = 0x80 */
        else if (LSE_Delay >= LSE_FAIL_FLAG) {
            if (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {
                /* Set flag: LSE FAIL */
                LSE_Delay |= LSE_FAIL_FLAG;
                led_ld4_on();
            }
            /* Disable LSE */
            RCC_LSEConfig(RCC_LSE_OFF);
            break;
        }
    }
}

int main(
    void)
{
    struct itimer Blink_Timer;

    /*At this stage the microcontroller clock setting is already configured,
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    led_init();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
        RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE,
        ENABLE);
    timer_init();
    lse_init();
    led_init();
    rs485_init();
    bacnet_init();
    timer_interval_start(&Blink_Timer, 125);
    for (;;) {
        if (timer_interval_expired(&Blink_Timer)) {
            timer_interval_reset(&Blink_Timer);
            led_ld3_toggle();
        }
        led_task();
        bacnet_task();
    }
}
