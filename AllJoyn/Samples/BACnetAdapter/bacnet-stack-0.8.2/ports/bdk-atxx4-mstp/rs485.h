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
*********************************************************************/
#ifndef RS485_H
#define RS485_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    void rs485_init(
        void);
    void rs485_rts_enable(
        bool enable);
    bool rs485_byte_available(
        uint8_t * data_register);
    bool rs485_receive_error(
        void);
    void rs485_bytes_send(
        uint8_t * buffer,       /* data to send */
        uint16_t nbytes);       /* number of bytes of data */
    uint32_t rs485_baud_rate(
        void);
    bool rs485_baud_rate_set(
        uint32_t baud);

    void rs485_turnaround_delay(
        void);
    void rs485_silence_time_reset(
        void);
    bool rs485_silence_time_elapsed(
        uint16_t milliseconds);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
