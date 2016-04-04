/**************************************************************************
*
* Copyright (C) 2012 Steve Karg <skarg@users.sourceforge.net>
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
#ifndef CSV_H
#define CSV_H

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "bacerror.h"
#include "rp.h"
#include "wp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    void CharacterString_Value_Property_Lists(
        const int **pRequired,
        const int **pOptional,
        const int **pProprietary);

    bool CharacterString_Value_Valid_Instance(
        uint32_t object_instance);
    unsigned CharacterString_Value_Count(
        void);
    uint32_t CharacterString_Value_Index_To_Instance(
        unsigned index);
    unsigned CharacterString_Value_Instance_To_Index(
        uint32_t instance);

    int CharacterString_Value_Read_Property(
        BACNET_READ_PROPERTY_DATA * rpdata);

    bool CharacterString_Value_Write_Property(
        BACNET_WRITE_PROPERTY_DATA * wp_data);

    /* optional API */
    bool CharacterString_Value_Object_Instance_Add(
        uint32_t instance);

    bool CharacterString_Value_Object_Name(
        uint32_t object_instance,
        BACNET_CHARACTER_STRING * object_name);
    bool CharacterString_Value_Name_Set(
        uint32_t object_instance,
        char *new_name);

    bool CharacterString_Value_Present_Value(
        uint32_t object_instance,
        BACNET_CHARACTER_STRING * value);
    bool CharacterString_Value_Present_Value_Set(
        uint32_t object_instance,
        BACNET_CHARACTER_STRING * value);
    bool CharacterString_Value_Description_Set(
        uint32_t object_instance,
        char *text_string);
    bool CharacterString_Value_Out_Of_Service(
        uint32_t object_instance);

    void CharacterString_Value_Init(
        void);


#ifdef TEST
#include "ctest.h"
    void testCharacterStringValue(
        Test * pTest);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
