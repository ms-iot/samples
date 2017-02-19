/**************************************************************************
*
* Copyright (C) 2010 Steve Karg <skarg@users.sourceforge.net>
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
#ifndef BACNET_NAME_H
#define BACNET_NAME_H

#include <stdint.h>
#include "bacstr.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    bool bacnet_name_set(
        uint16_t eeprom_offset,
        BACNET_CHARACTER_STRING * char_string);
    void bacnet_name_init(
        uint16_t eeprom_offset,
        char *default_string);
    bool bacnet_name_save(
        uint16_t offset,
        uint8_t encoding,
        char *str,
        uint8_t length);
    void bacnet_name(
        uint16_t eeprom_offset,
        BACNET_CHARACTER_STRING * char_string,
        char *default_string);
    bool bacnet_name_write_unique(
        uint16_t offset,
        int object_type,
        uint32_t object_instance,
        BACNET_CHARACTER_STRING * char_string,
        BACNET_ERROR_CLASS * error_class,
        BACNET_ERROR_CODE * error_code);
    /* no required minumum length or duplicate checking */
    bool bacnet_name_write(
        uint16_t offset,
        BACNET_CHARACTER_STRING * char_string,
        BACNET_ERROR_CLASS * error_class,
        BACNET_ERROR_CODE * error_code);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
