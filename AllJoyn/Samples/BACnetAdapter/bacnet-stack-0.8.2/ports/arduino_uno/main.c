/**
 * @file
 * @author Miguel Fernandes <miguelandre.fernandes@gmail.com>
 * @date 6 de Jun de 2013
 * @brief BACnet/IP for Wiznet on Arduino-Uno
 *
 * This port is for BACnet/ip and uses part of the Arduino Ethernet
 * library so it needs the stock Arduino Etherenet Shield
 * (the one with the W5100 chip). The port was done by writting a C
 * wrapper around the c++ Ethernet library and adapting the
 * existing port for Atmega168 (mainly functions bip.c and bip-init.c)
 * to use the wrapper functions. The port also needs Arduino core and
 * Ethernet libraries to compile.
 */
#include <stdbool.h>
#include <stdint.h>
#include "datalink.h"
#include "npdu.h"
#include "handlers.h"
#include "txbuf.h"
#include "iam.h"
#include "device.h"
#include "av.h"
#include "uart.h"
#include "w5100Wrapper.h"
#include "Arduino.h"
#include <avr/io.h>
#define BAUD 9600
#include <util/setbaud.h>

/* From the WhoIs hander - performed by the DLMSTP module */
extern bool Send_I_Am_Flag;
/* local version override */
const char *BACnet_Version = "1.0";
static uint8_t Ethernet_MAC_Address[MAX_MAC_LEN] =
    { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
uint8_t ipAddress[] = { 192, 168, 0, 185 };
uint8_t gateway[] = { 192, 168, 0, 1 };
uint8_t netmask[] = { 255, 255, 255, 0 };

FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);
FILE uart_io = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

/* For porting to IAR, see:
   http://www.avrfreaks.net/wiki/index.php/Documentation:AVR_GCC/IarToAvrgcc*/

/* dummy function - so we can use default demo handlers */
bool dcc_communication_enabled(void)
{
    return true;
}

void setup()
{
    //INIT W5100
    init_func(CW5100Class_new());
    setMACAddress_func(CW5100Class_new(), Ethernet_MAC_Address);
    setIPAddress_func(CW5100Class_new(), ipAddress);
    setGatewayIp_func(CW5100Class_new(), gateway);
    setSubnetMask_func(CW5100Class_new(), netmask);

    uart_init();
    stdout = &uart_output;
    stdin = &uart_input;
    stderr = &uart_output;

#ifdef DEBUG
    fprintf(stderr, "Starting BACNET application..\n");
#endif

}

static uint8_t PDUBuffer[MAX_MPDU];
int main(void)
{
    uint16_t pdu_len = 0;
    BACNET_ADDRESS src; /* source address */

    init();

    setup();

    datalink_init(NULL);
    for (;;) {

        /* other tasks */
        /* BACnet handling */
        pdu_len = datalink_receive(&src, &PDUBuffer[0], sizeof(PDUBuffer), 0);
        if (pdu_len) {
            npdu_handler(&src, &PDUBuffer[0], pdu_len);
        }
    }
}
