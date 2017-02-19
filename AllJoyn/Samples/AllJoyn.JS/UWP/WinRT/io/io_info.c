/**
 * @file
 */
/******************************************************************************
 * Copyright AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

#include "ajs.h"
#include "ajs_io.h"

#define GPIO "GPIO"
#define I2C  "I2C"
#define SPI  "SPI"
#define CS   "CS"

uint8_t gpioTextLen = ArraySize(GPIO)-1;
uint8_t csTextLen = ArraySize(CS)-1;


#ifndef REF_BOARD
#define REF_BOARD RPI2
#endif

#if (REF_BOARD == RPI || REF_BOARD == RPI2)      // Raspberry Pie 2
static const AJS_IO_Info info[] = {
    { 0, 0, "", "", "NA" },
    { 0, 1, "", "", "3.3V PWR" },
    { 0, 2, "", "", "5V PWR" },
    { AJS_IO_FUNCTION_DIGITAL_IO| AJS_IO_FUNCTION_I2C_SDA, 3, I2C"1", "", "I2C1 SDA" },
    { 0, 4, "", "", "5V PWR" },
    { AJS_IO_FUNCTION_DIGITAL_IO| AJS_IO_FUNCTION_I2C_SCL, 5, I2C"1", "", "I2C1 SCL" },
    { 0, 6, "", "", "GND" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 7,  GPIO"4", "", "GPIO 4" },
    { 0, 8, "", "", "Reserved" },
    { 0, 9, "", "", "GND" },
    { 0, 10, "", "", "Reserved" },
    { AJS_IO_FUNCTION_DIGITAL_IO | AJS_IO_FUNCTION_SPI_SS, 11, SPI"1", CS"0", "SPI1 CS0" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 12, GPIO"18", "", "GPIO 18" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 13, GPIO"27", "", "GPIO 27" },
    { 0, 14, "", "", "GND" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 15, GPIO"22", "", "GPIO 22" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 16, GPIO"23", "", "GPIO 23" },
    { 0, 17, "", "", "3.3V PWR" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 18, GPIO"24", "", "GPIO 24" },
    { AJS_IO_FUNCTION_DIGITAL_IO | AJS_IO_FUNCTION_SPI_MOSI, 19, SPI"0", "", "SPI0 MOSI" },
    { 0, 20,  "",   "",  "GND" },
    { AJS_IO_FUNCTION_DIGITAL_IO | AJS_IO_FUNCTION_SPI_MISO, 21, SPI"0", "", "SPI0 MISO" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 22,  GPIO"25",   "",  "GPIO 25" },
    { AJS_IO_FUNCTION_DIGITAL_IO | AJS_IO_FUNCTION_SPI_SCK, 23, SPI"0", "", "SPI0 SCLK" },
    { AJS_IO_FUNCTION_DIGITAL_IO | AJS_IO_FUNCTION_SPI_SS, 24, SPI"0", CS"0", "SPI0 CS0" },
    { 0, 25, "", "", "GND" },
    { AJS_IO_FUNCTION_DIGITAL_IO | AJS_IO_FUNCTION_SPI_SS, 26, SPI"0", CS"1", "SPI0 CS1" },
    { 0, 27, "", "", "Reserved" },
    { 0, 28, "", "", "Reserved" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 29, GPIO"5", "", "GPIO 5" },
    { 0, 30,  "",   "",  "GND" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 31, GPIO"6", "", "GPIO 6" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 32, GPIO"12", "", "GPIO 12" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 33, GPIO"13", "", "GPIO 13" },
    { 0, 34, "", "", "GND" },
    { AJS_IO_FUNCTION_DIGITAL_IO | AJS_IO_FUNCTION_SPI_MISO, 35, SPI"1", "", "SPI1 MISO" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 36, GPIO"16", "", "GPIO 16" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 37, GPIO"26", "", "GPIO 26" },
    { AJS_IO_FUNCTION_DIGITAL_IO | AJS_IO_FUNCTION_SPI_MOSI, 38, SPI"1", "", "SPI1 MOSI" },
    { 0, 39, "", "", "GND" },
    { AJS_IO_FUNCTION_DIGITAL_IO | AJS_IO_FUNCTION_SPI_SCK, 40, SPI"1", "", "SPI1 SCLK" }
};

#elif (REF_BOARD == MBM)

static const AJS_IO_Info info[] = {
    { 0, 0, "", "", "NA"},
    { 0, 1, "", "", "GND" },
    { 0, 2, "", "", "GND" },
    { 0, 3, "", "", "5V PWR" },
    { 0, 4, "", "", "3.3V PWR" },
    { AJS_IO_FUNCTION_SPI_SS, 5, SPI"0", CS"0", "SPI0 CS0" },
    { AJS_IO_FUNCTION_UART_TX, 6, "UART1", "", "UART1 TX" },
    { AJS_IO_FUNCTION_SPI_MOSI, 7, SPI"0", "", "SPI0 MOSI" },
    { AJS_IO_FUNCTION_UART_RX, 8, "UART1", "", "UART1 RX" },
    { AJS_IO_FUNCTION_SPI_MISO, 9, SPI"0", "", "SPI0 MISO" },
    { 0, 10,  "UART1", "", "UART1 CTS" },
    { AJS_IO_FUNCTION_SPI_SCK, 11, SPI"0", "", "SPI0 SCLK" },
    { 0, 12,  "UART1", "", "UART1 RTS" },
    { AJS_IO_FUNCTION_I2C_SCL, 13, I2C"5", "", "I2C5 SCL" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 14, GPIO"3", "", "GPIO 3" },
    { AJS_IO_FUNCTION_I2C_SDA, 15, I2C"5", "", "I2C5 SDA" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 16, GPIO"4", "", "GPIO 4" },
    { AJS_IO_FUNCTION_UART_TX, 17, "UART2", "", "UART2 TX" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 18, GPIO"5", "", "GPIO 5" },
    { AJS_IO_FUNCTION_UART_RX, 19, "UART2", "", "UART2 RX" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 20, GPIO"6", "", "GPIO 6" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 21, GPIO"0", "", "GPIO 0" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 22, GPIO"7", "", "GPIO 7" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 23, GPIO"1", "", "GPIO 1" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 24, GPIO"8", "", "GPIO 8" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 25, GPIO"2", "", "GPIO 2" },
    { AJS_IO_FUNCTION_DIGITAL_IO, 26, GPIO"9", "", "GPIO 9" }
};

#endif

uint16_t AJS_TargetIO_GetNumPins()
{
    return ArraySize(info);
}

const AJS_IO_Info* AJS_TargetIO_GetInfo(uint16_t pin)
{
    if (pin < ArraySize(info)) {
        return &info[pin];
    } else {
        return NULL;
    }
}
