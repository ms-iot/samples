/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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
#ifndef MSO_H
#define MSO_H

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "bacerror.h"
#include "rp.h"
#include "wp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    void Multistate_Output_Property_Lists(
        const int **pRequired,
        const int **pOptional,
        const int **pProprietary);
    bool Multistate_Output_Valid_Instance(
        uint32_t object_instance);
    unsigned Multistate_Output_Count(
        void);
    uint32_t Multistate_Output_Index_To_Instance(
        unsigned index);
    unsigned Multistate_Output_Instance_To_Index(
        uint32_t object_instance);

    bool Multistate_Output_Object_Name(
        uint32_t object_instance,
        BACNET_CHARACTER_STRING * object_name);

    void Multistate_Output_Init(
        void);

    int Multistate_Output_Read_Property(
        BACNET_READ_PROPERTY_DATA * rpdata);

    bool Multistate_Output_Write_Property(
        BACNET_WRITE_PROPERTY_DATA * wp_data);

    bool Multistate_Output_Change_Of_Value(
        uint32_t instance);
    void Multistate_Output_Change_Of_Value_Clear(
        uint32_t instance);
    bool Multistate_Output_Encode_Value_List(
        uint32_t object_instance,
        BACNET_PROPERTY_VALUE * value_list);

    uint32_t Multistate_Output_Present_Value(
        uint32_t instance);
    bool Multistate_Output_Present_Value_Set(
        uint32_t instance,
        unsigned value,
        unsigned priority);
    bool Multistate_Output_Present_Value_Relinquish(
        uint32_t instance,
        unsigned priority);

    bool Multistate_Output_Out_Of_Service(
        uint32_t instance);
    void Multistate_Output_Out_Of_Service_Set(
        uint32_t instance,
        bool value);

    char *Multistate_Output_Description(
        uint32_t instance);
    bool Multistate_Output_Description_Set(
        uint32_t object_instance,
        char *text_string);

    bool Multistate_Output_State_Text_Set(
        uint32_t object_instance,
        uint32_t state_index,
        char *new_name);
    bool Multistate_Output_Max_States_Set(
        uint32_t instance,
        uint32_t max_states_requested);
    char *Multistate_Output_State_Text(
        uint32_t object_instance,
        uint32_t state_index);


#ifdef TEST
#include "ctest.h"
    void testMultistateOutput(
        Test * pTest);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
