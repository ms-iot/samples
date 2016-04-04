/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
* Copyright (C) 2011 Krzysztof Malorny <malornykrzysztof@gmail.com>
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
#ifndef AI_H
#define AI_H

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "rp.h"
#include "wp.h"
#if defined(INTRINSIC_REPORTING)
#include "nc.h"
#include "getevent.h"
#include "alarm_ack.h"
#include "get_alarm_sum.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    typedef struct analog_input_descr {
        unsigned Event_State:3;
        float Present_Value;
        BACNET_RELIABILITY Reliability;
        bool Out_Of_Service;
        uint8_t Units;
        float Prior_Value;
        float COV_Increment;
        bool Changed;
#if defined(INTRINSIC_REPORTING)
        uint32_t Time_Delay;
        uint32_t Notification_Class;
        float High_Limit;
        float Low_Limit;
        float Deadband;
        unsigned Limit_Enable:2;
        unsigned Event_Enable:3;
        unsigned Notify_Type:1;
        ACKED_INFO Acked_Transitions[MAX_BACNET_EVENT_TRANSITION];
        BACNET_DATE_TIME Event_Time_Stamps[MAX_BACNET_EVENT_TRANSITION];
        /* time to generate event notification */
        uint32_t Remaining_Time_Delay;
        /* AckNotification informations */
        ACK_NOTIFICATION Ack_notify_data;
#endif
    } ANALOG_INPUT_DESCR;

    void Analog_Input_Property_Lists(
        const int **pRequired,
        const int **pOptional,
        const int **pProprietary);

    bool Analog_Input_Valid_Instance(
        uint32_t object_instance);
    unsigned Analog_Input_Count(
        void);
    uint32_t Analog_Input_Index_To_Instance(
        unsigned index);
    unsigned Analog_Input_Instance_To_Index(
        uint32_t instance);
    bool Analog_Input_Object_Instance_Add(
        uint32_t instance);

    bool Analog_Input_Object_Name(
        uint32_t object_instance,
        BACNET_CHARACTER_STRING * object_name);
    bool Analog_Input_Name_Set(
        uint32_t object_instance,
        char *new_name);

    char *Analog_Input_Description(
        uint32_t instance);
    bool Analog_Input_Description_Set(
        uint32_t instance,
        char *new_name);

    bool Analog_Input_Units_Set(
        uint32_t instance,
        uint16_t units);
    uint16_t Analog_Input_Units(
        uint32_t instance);

    int Analog_Input_Read_Property(
        BACNET_READ_PROPERTY_DATA * rpdata);
    bool Analog_Input_Write_Property(
        BACNET_WRITE_PROPERTY_DATA * wp_data);

    float Analog_Input_Present_Value(
        uint32_t object_instance);
    void Analog_Input_Present_Value_Set(
        uint32_t object_instance,
        float value);

    bool Analog_Input_Out_Of_Service(
        uint32_t object_instance);
    void Analog_Input_Out_Of_Service_Set(
        uint32_t object_instance,
        bool oos_flag);

    bool Analog_Input_Change_Of_Value(
        uint32_t instance);
    void Analog_Input_Change_Of_Value_Clear(
        uint32_t instance);
    bool Analog_Input_Encode_Value_List(
        uint32_t object_instance,
        BACNET_PROPERTY_VALUE * value_list);
    float Analog_Input_COV_Increment(
        uint32_t instance);
    void Analog_Input_COV_Increment_Set(
        uint32_t instance,
        float value);

    /* note: header of Intrinsic_Reporting function is required
       even when INTRINSIC_REPORTING is not defined */
    void Analog_Input_Intrinsic_Reporting(
        uint32_t object_instance);

#if defined(INTRINSIC_REPORTING)
    int Analog_Input_Event_Information(
        unsigned index,
        BACNET_GET_EVENT_INFORMATION_DATA * getevent_data);

    int Analog_Input_Alarm_Ack(
        BACNET_ALARM_ACK_DATA * alarmack_data,
        BACNET_ERROR_CODE * error_code);

    int Analog_Input_Alarm_Summary(
        unsigned index,
        BACNET_GET_ALARM_SUMMARY_DATA * getalarm_data);
#endif

    void Analog_Input_Init(
        void);

#ifdef TEST
#include "ctest.h"
    void testAnalogInput(
        Test * pTest);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
