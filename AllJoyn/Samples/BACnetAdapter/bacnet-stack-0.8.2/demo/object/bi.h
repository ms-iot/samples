/**************************************************************************
*
* Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
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
#ifndef BI_H
#define BI_H

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "cov.h"
#include "rp.h"
#include "wp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    void Binary_Input_Property_Lists(
        const int **pRequired,
        const int **pOptional,
        const int **pProprietary);

    bool Binary_Input_Valid_Instance(
        uint32_t object_instance);
    unsigned Binary_Input_Count(
        void);
    uint32_t Binary_Input_Index_To_Instance(
        unsigned index);
    unsigned Binary_Input_Instance_To_Index(
        uint32_t instance);
    bool Binary_Input_Object_Instance_Add(
        uint32_t instance);

    bool Binary_Input_Object_Name(
        uint32_t object_instance,
        BACNET_CHARACTER_STRING * object_name);
    bool Binary_Input_Name_Set(
        uint32_t object_instance,
        char *new_name);

    BACNET_BINARY_PV Binary_Input_Present_Value(
        uint32_t object_instance);
    bool Binary_Input_Present_Value_Set(
        uint32_t object_instance,
        BACNET_BINARY_PV value);

    char *Binary_Input_Description(
        uint32_t instance);
    bool Binary_Input_Description_Set(
        uint32_t instance,
        char *new_name);

    char *Binary_Input_Inactive_Text(
        uint32_t instance);
    bool Binary_Input_Inactive_Text_Set(
        uint32_t instance,
        char *new_name);
    char *Binary_Input_Active_Text(
        uint32_t instance);
    bool Binary_Input_Active_Text_Set(
        uint32_t instance,
        char *new_name);

    BACNET_POLARITY Binary_Input_Polarity(
        uint32_t object_instance);
    bool Binary_Input_Polarity_Set(
        uint32_t object_instance,
        BACNET_POLARITY polarity);

    bool Binary_Input_Out_Of_Service(
        uint32_t object_instance);
    void Binary_Input_Out_Of_Service_Set(
        uint32_t object_instance,
        bool value);

    bool Binary_Input_Encode_Value_List(
        uint32_t object_instance,
        BACNET_PROPERTY_VALUE * value_list);
    bool Binary_Input_Change_Of_Value(
        uint32_t instance);
    void Binary_Input_Change_Of_Value_Clear(
        uint32_t instance);

    int Binary_Input_Read_Property(
        BACNET_READ_PROPERTY_DATA * rpdata);

    bool Binary_Input_Write_Property(
        BACNET_WRITE_PROPERTY_DATA * wp_data);
    void Binary_Input_Init(
        void);

#ifdef TEST
#include "ctest.h"
    void testBinaryInput(
        Test * pTest);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
