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
#ifndef AO_H
#define AO_H

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "bacerror.h"
#include "rp.h"
#include "wp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    void Analog_Output_Property_Lists(
        const int **pRequired,
        const int **pOptional,
        const int **pProprietary);
    bool Analog_Output_Valid_Instance(
        uint32_t object_instance);
    unsigned Analog_Output_Count(
        void);
    uint32_t Analog_Output_Index_To_Instance(
        unsigned index);
    unsigned Analog_Output_Instance_To_Index(
        uint32_t instance);
    bool Analog_Output_Object_Instance_Add(
        uint32_t instance);

    float Analog_Output_Present_Value(
        uint32_t object_instance);
    unsigned Analog_Output_Present_Value_Priority(
        uint32_t object_instance);
    bool Analog_Output_Present_Value_Set(
        uint32_t object_instance,
        float value,
        unsigned priority);
    bool Analog_Output_Present_Value_Relinquish(
        uint32_t object_instance,
        unsigned priority);

    float Analog_Output_Relinquish_Default(
        uint32_t object_instance);
    bool Analog_Output_Relinquish_Default_Set(
        uint32_t object_instance,
        float value);

    bool Analog_Output_Change_Of_Value(
        uint32_t instance);
    void Analog_Output_Change_Of_Value_Clear(
        uint32_t instance);
    bool Analog_Output_Encode_Value_List(
        uint32_t object_instance,
        BACNET_PROPERTY_VALUE * value_list);
    float Analog_Output_COV_Increment(
        uint32_t instance);
    void Analog_Output_COV_Increment_Set(
        uint32_t instance,
        float value);

    bool Analog_Output_Object_Name(
        uint32_t object_instance,
        BACNET_CHARACTER_STRING * object_name);
    bool Analog_Output_Name_Set(
        uint32_t object_instance,
        char *new_name);

    char *Analog_Output_Description(
        uint32_t instance);
    bool Analog_Output_Description_Set(
        uint32_t instance,
        char *new_name);

    bool Analog_Output_Units_Set(
        uint32_t instance,
        uint16_t units);
    uint16_t Analog_Output_Units(
        uint32_t instance);

    bool Analog_Output_Out_Of_Service(
        uint32_t instance);
    void Analog_Output_Out_Of_Service_Set(
        uint32_t instance,
        bool oos_flag);

    void Analog_Output_Init(
        void);

    int Analog_Output_Read_Property(
        BACNET_READ_PROPERTY_DATA * rpdata);
    bool Analog_Output_Write_Property(
        BACNET_WRITE_PROPERTY_DATA * wp_data);

#ifdef TEST
#include "ctest.h"
    void testAnalogOutput(
        Test * pTest);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
