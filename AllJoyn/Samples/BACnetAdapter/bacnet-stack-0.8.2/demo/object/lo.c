/**
* @file
* @author Steve Karg
* @date 2013
* @brief Lighting Output object
*
* @section LICENSE
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
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "bacapp.h"
#include "config.h"     /* the custom stuff */
#include "rp.h"
#include "wp.h"
#include "lighting.h"
#include "handlers.h"
#include "proplist.h"
/* me! */
#include "lo.h"

#ifndef MAX_LIGHTING_OUTPUTS
#define MAX_LIGHTING_OUTPUTS 8
#endif

struct lighting_output_object {
    float Present_Value;
    float Tracking_Value;
    float Physical_Value;
    BACNET_LIGHTING_COMMAND Lighting_Command;
    BACNET_LIGHTING_IN_PROGRESS In_Progress;
    bool Out_Of_Service:1;
    bool Blink_Warn_Enable:1;
    bool Egress_Active:1;
    uint32_t Egress_Time;
    uint32_t Default_Fade_Time;
    float Default_Ramp_Rate;
    float Default_Step_Increment;
    BACNET_LIGHTING_TRANSITION Transition;
    float Feedback_Value;
    float Priority_Array[BACNET_MAX_PRIORITY];
    uint16_t Priority_Active_Bits;
    float Relinquish_Default;
    float Power;
    float Instantaneous_Power;
    float Min_Actual_Value;
    float Max_Actual_Value;
    uint8_t Lighting_Command_Default_Priority;
};
struct lighting_output_object Lighting_Output[MAX_LIGHTING_OUTPUTS];

/* These arrays are used by the ReadPropertyMultiple handler and
   property-list property (as of protocol-revision 14) */
static const int Lighting_Output_Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_PRESENT_VALUE,
    PROP_TRACKING_VALUE,
    PROP_LIGHTING_COMMAND,
    PROP_IN_PROGRESS,
    PROP_STATUS_FLAGS,
    PROP_OUT_OF_SERVICE,
    PROP_BLINK_WARN_ENABLE,
    PROP_EGRESS_TIME,
    PROP_EGRESS_ACTIVE,
    PROP_DEFAULT_FADE_TIME,
    PROP_DEFAULT_RAMP_RATE,
    PROP_DEFAULT_STEP_INCREMENT,
    PROP_PRIORITY_ARRAY,
    PROP_RELINQUISH_DEFAULT,
    PROP_LIGHTING_COMMAND_DEFAULT_PRIORITY,
    -1
};
static const int Lighting_Output_Properties_Optional[] = {
    -1
};

static const int Lighting_Output_Properties_Proprietary[] = {
    -1
};

/**
 * Returns the list of required, optional, and proprietary properties.
 * Used by ReadPropertyMultiple service.
 *
 * @param pRequired - pointer to list of int terminated by -1, of
 * BACnet required properties for this object.
 * @param pOptional - pointer to list of int terminated by -1, of
 * BACnet optkional properties for this object.
 * @param pProprietary - pointer to list of int terminated by -1, of
 * BACnet proprietary properties for this object.
 */
void Lighting_Output_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
    if (pRequired)
        *pRequired = Lighting_Output_Properties_Required;
    if (pOptional)
        *pOptional = Lighting_Output_Properties_Optional;
    if (pProprietary)
        *pProprietary = Lighting_Output_Properties_Proprietary;

    return;
}

/**
 * Determines if a given Lighting Output instance is valid
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return  true if the instance is valid, and false if not
 */
bool Lighting_Output_Valid_Instance(
    uint32_t object_instance)
{
    unsigned int index;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        return true;
    }

    return false;
}

/**
 * Determines the number of Lighting Output objects
 *
 * @return  Number of Lighting Output objects
 */
unsigned Lighting_Output_Count(
    void)
{
    return MAX_LIGHTING_OUTPUTS;
}

/**
 * Determines the object instance-number for a given 0..N index
 * of Lighting Output objects where N is Lighting_Output_Count().
 *
 * @param  index - 0..MAX_LIGHTING_OUTPUTS value
 *
 * @return  object instance-number for the given index
 */
uint32_t Lighting_Output_Index_To_Instance(
    unsigned index)
{
    uint32_t instance = 1;

    instance += index;

    return instance;
}

/**
 * For a given object instance-number, determines a 0..N index
 * of Lighting Output objects where N is Lighting_Output_Count().
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return  index for the given instance-number, or MAX_LIGHTING_OUTPUTS
 * if not valid.
 */
unsigned Lighting_Output_Instance_To_Index(
    uint32_t object_instance)
{
    unsigned index = MAX_LIGHTING_OUTPUTS;

    if (object_instance) {
        index = object_instance - 1;
        if (index > MAX_LIGHTING_OUTPUTS) {
            index = MAX_LIGHTING_OUTPUTS;
        }
    }

    return index;
}

/**
 * For a given object instance-number, determines the present-value
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return  present-value of the object
 */
float Lighting_Output_Present_Value(
    uint32_t object_instance)
{
    float value = 0.0;
    unsigned index = 0;
    unsigned p = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        value = Lighting_Output[index].Relinquish_Default;
        for (p = 0; p < BACNET_MAX_PRIORITY; p++) {
            if (BIT_CHECK(Lighting_Output[index].Priority_Active_Bits, p)) {
                value = Lighting_Output[index].Priority_Array[p];
                break;
            }
        }
    }

    return value;
}

/**
 * For a given object instance-number, determines the value at a
 * given priority.
 *
 * @param  object_instance - object-instance number of the object
 * @param  priority - priority 1..16
 *
 * @return  priority-value of the object
 */
static float Lighting_Output_Priority_Value(
    uint32_t object_instance,
    unsigned priority)
{
    float value = 0.0;
    unsigned index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        if (priority && (priority <= BACNET_MAX_PRIORITY)) {
            priority--;
            value = Lighting_Output[index].Priority_Array[priority];
        }
    }

    return value;
}

/**
 * For a given object instance-number, determines if the given priority
 * is active or NULL.
 *
 * @param  object_instance - object-instance number of the object
 * @param  priority - priority 1..16
 *
 * @return  true if the priority slot is active
 */
static bool Lighting_Output_Priority_Active(
    uint32_t object_instance,
    unsigned priority)
{
    bool status = false;
    unsigned index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        if (priority && (priority <= BACNET_MAX_PRIORITY)) {
            priority--;
            if (BIT_CHECK(Lighting_Output[index].Priority_Active_Bits, priority)) {
                status = true;
            }
        }
    }

    return status;
}

/**
 * For a given object instance-number, determines the active priority
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return  active priority 1..16, or 0 if no priority is active
 */
unsigned Lighting_Output_Present_Value_Priority(
    uint32_t object_instance)
{
    unsigned index = 0; /* instance to index conversion */
    unsigned p = 0;     /* loop counter */
    unsigned priority = 0;      /* return value */

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        for (p = 0; p < BACNET_MAX_PRIORITY; p++) {
            if (BIT_CHECK(Lighting_Output[index].Priority_Active_Bits, p)) {
                priority = p + 1;
                break;
            }
        }
    }

    return priority;
}

/**
 * For a given object instance-number, sets the present-value at a given
 * priority 1..16.
 *
 * @param  object_instance - object-instance number of the object
 * @param  value - floating point analog value
 * @param  priority - priority 1..16
 *
 * @return  true if values are within range and present-value is set.
 */
bool Lighting_Output_Present_Value_Set(
    uint32_t object_instance,
    float value,
    unsigned priority)
{
    unsigned index = 0;
    bool status = false;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        if (priority && (priority <= BACNET_MAX_PRIORITY) &&
            (priority != 6 /* reserved */ )) {
            priority--;
            BIT_SET(Lighting_Output[index].Priority_Active_Bits, priority);
            Lighting_Output[index].Priority_Array[priority] = value;
            status = true;
        }
    }

    return status;
}

/**
 * For a given object instance-number, relinquishes the present-value
 * at a given priority 1..16.
 *
 * @param  object_instance - object-instance number of the object
 * @param  priority - priority 1..16
 *
 * @return  true if values are within range and present-value is set.
 */
bool Lighting_Output_Present_Value_Relinquish(
    uint32_t object_instance,
    unsigned priority)
{
    unsigned index = 0;
    bool status = false;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        if (priority && (priority <= BACNET_MAX_PRIORITY) &&
            (priority != 6 /* reserved */ )) {
            priority--;
            BIT_CLEAR(Lighting_Output[index].Priority_Active_Bits, priority);
            Lighting_Output[index].Priority_Array[priority] = 0.0;
            status = true;
        }
    }

    return status;
}

/**
 * For a given object instance-number, loads the object-name into
 * a characterstring. Note that the object name must be unique
 * within this device.
 *
 * @param  object_instance - object-instance number of the object
 * @param  object_name - holds the object-name retrieved
 *
 * @return  true if object-name was retrieved
 */
bool Lighting_Output_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    char text_string[32] = "";
    bool status = false;
    unsigned index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        sprintf(text_string, "LIGHTING OUTPUT %lu",
            (unsigned long) object_instance);
        status = characterstring_init_ansi(object_name, text_string);
    }

    return status;
}

/**
 * For a given object instance-number, sets the lighting-command.
 *
 * @param object_instance - object-instance number of the object
 * @param value - holds the lighting command value
 *
 * @return  true if lighting command was set
 */
bool Lighting_Output_Lighting_Command_Set(
    uint32_t object_instance,
    BACNET_LIGHTING_COMMAND *value)
{
    bool status = false;
    unsigned index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        // FIXME: check lighting command member values
        status = lighting_command_copy(
            &Lighting_Output[index].Lighting_Command,
            value);
        // FIXME: set all the other values, and get the light levels moving
    }

    return status;
}

/**
 * For a given object instance-number, gets the lighting-command.
 *
 * @param object_instance - object-instance number of the object
 * @param value - holds the lighting command value
 *
 * @return true if lighting command was retrieved
 */
bool Lighting_Output_Lighting_Command(
    uint32_t object_instance,
    BACNET_LIGHTING_COMMAND *value)
{
    bool status = false;
    unsigned index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        status = lighting_command_copy(value,
            &Lighting_Output[index].Lighting_Command);
    }

    return status;
}

/**
 * For a given object instance-number, gets the in-progress property value
 *
 * @param object_instance - object-instance number of the object
 *
 * @return the in-progress value of this object instance.
 */
BACNET_LIGHTING_IN_PROGRESS Lighting_Output_In_Progress(
    uint32_t object_instance)
{
    BACNET_LIGHTING_IN_PROGRESS value = BACNET_LIGHTING_IDLE;
    unsigned index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        value = Lighting_Output[index].In_Progress;
    }

    return value;
}

/**
 * For a given object instance-number, sets the in-progress value of the
 * object.
 *
 * @param object_instance - object-instance number of the object
 * @param in_progress - holds the value to be set
 *
 * @return true if value was set
 */
bool Lighting_Output_In_Progress_Set(
    uint32_t object_instance,
    BACNET_LIGHTING_IN_PROGRESS in_progress)
{
    bool status = false;
    unsigned index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        Lighting_Output[index].In_Progress = in_progress;

    }

    return status;
}

/**
 * For a given object instance-number, gets the tracking-value property
 *
 * @param object_instance - object-instance number of the object
 *
 * @return the tracking-value of this object instance.
 */
float Lighting_Output_Tracking_Value(
    uint32_t object_instance)
{
    float value = 0.0;
    unsigned index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        value = Lighting_Output[index].Tracking_Value;
    }

    return value;
}

/**
 * For a given object instance-number, sets the in-progress value of the
 * object.
 *
 * @param object_instance - object-instance number of the object
 * @param in_progress - holds the value to be set
 *
 * @return true if value was set
 */
bool Lighting_Output_Tracking_Value_Set(
    uint32_t object_instance,
    float value)
{
    bool status = false;
    unsigned int index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        Lighting_Output[index].Tracking_Value = value;
        status = true;
    }

    return status;
}

/**
 * For a given object instance-number, gets the blink-warn-enable
 * property value
 *
 * @param object_instance - object-instance number of the object
 *
 * @return the blink-warn-enable property value of this object
 */
bool Lighting_Output_Blink_Warn_Enable(
    uint32_t object_instance)
{
    bool value = false;
    unsigned index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        value = Lighting_Output[index].Blink_Warn_Enable;
    }

    return value;
}

/**
 * For a given object instance-number, sets the blink-warn-enable
 * property value in the object.
 *
 * @param object_instance - object-instance number of the object
 * @param enable - holds the value to be set
 *
 * @return true if value was set
 */
bool Lighting_Output_Blink_Warn_Enable_Set(
    uint32_t object_instance,
    bool enable)
{
    bool status = false;
    unsigned int index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        Lighting_Output[index].Blink_Warn_Enable = enable;
        status = true;
    }

    return status;
}

/**
 * For a given object instance-number, gets the egress-time
 * property value
 *
 * @param object_instance - object-instance number of the object
 *
 * @return the egress-time property value of this object
 */
uint32_t Lighting_Output_Egress_Time(
    uint32_t object_instance)
{
    uint32_t value = 0;
    unsigned int index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        value = Lighting_Output[index].Egress_Time;
    }

    return value;
}

/**
 * For a given object instance-number, sets the egress-time
 * property value of the object.
 *
 * @param object_instance - object-instance number of the object
 * @param seconds - holds the value to be set
 *
 * @return true if value was set
 */
bool Lighting_Output_Egress_Time_Set(
    uint32_t object_instance,
    uint32_t seconds)
{
    bool status = false;
    unsigned int index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        Lighting_Output[index].Egress_Time = seconds;
        status = true;
    }

    return status;
}

/**
 * For a given object instance-number, gets the egress-active
 * property value
 *
 * @param object_instance - object-instance number of the object
 *
 * @return the egress-active property value of this object
 */
bool Lighting_Output_Egress_Active(
    uint32_t object_instance)
{
    bool value = false;
    unsigned int index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        value = Lighting_Output[index].Egress_Active;
    }

    return value;
}

/**
 * For a given object instance-number, gets the fade-time
 * property value
 *
 * @param object_instance - object-instance number of the object
 *
 * @return the fade-time property value of this object
 */
uint32_t Lighting_Output_Default_Fade_Time(
    uint32_t object_instance)
{
    uint32_t value = 0;
    unsigned int index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        value = Lighting_Output[index].Default_Fade_Time;
    }

    return value;
}

/**
 * For a given object instance-number, sets the fade-time
 * property value of the object.
 *
 * @param object_instance - object-instance number of the object
 * @param milliseconds - holds the value to be set
 *
 * @return true if value was set
 */
bool Lighting_Output_Default_Fade_Time_Set(
    uint32_t object_instance,
    uint32_t milliseconds)
{
    bool status = false;
    unsigned int index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if ((index < MAX_LIGHTING_OUTPUTS) &&
        (milliseconds >= 100) &&
        (milliseconds <= 86400000)) {
        Lighting_Output[index].Default_Fade_Time = milliseconds;
        status = true;
    }

    return status;
}

/**
 * For a given object instance-number, gets the ramp-rate
 * property value
 *
 * @param object_instance - object-instance number of the object
 *
 * @return the ramp-rate property value of this object
 */
float Lighting_Output_Default_Ramp_Rate(
    uint32_t object_instance)
{
    float value = 0.0;
    unsigned int index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        value = Lighting_Output[index].Default_Ramp_Rate;
    }

    return value;
}

/**
 * For a given object instance-number, sets the ramp-rate value of the
 * object.
 *
 * @param object_instance - object-instance number of the object
 * @param percent_per_second - holds the value to be set
 *
 * @return true if value was set
 */
bool Lighting_Output_Default_Ramp_Rate_Set(
    uint32_t object_instance,
    float percent_per_second)
{
    bool status = false;
    unsigned int index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if ((index < MAX_LIGHTING_OUTPUTS) &&
        (percent_per_second >= 0.1) &&
        (percent_per_second <= 100.0)) {
        Lighting_Output[index].Default_Ramp_Rate = percent_per_second;
        status = true;
    }

    return status;
}

/**
 * For a given object instance-number, gets the default-step-increment
 * property value
 *
 * @param object_instance - object-instance number of the object
 *
 * @return the default-step-increment property value of this object
 */
float Lighting_Output_Default_Step_Increment(
    uint32_t object_instance)
{
    float value = 0.0;
    unsigned int index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        value = Lighting_Output[index].Default_Step_Increment;
    }

    return value;
}

/**
 * For a given object instance-number, sets the default-step-increment
 * property value of the object.
 *
 * @param object_instance - object-instance number of the object
 * @param step_increment - holds the value to be set
 *
 * @return true if value was set
 */
bool Lighting_Output_Default_Step_Increment_Set(
    uint32_t object_instance,
    float step_increment)
{
    bool status = false;
    unsigned int index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if ((index < MAX_LIGHTING_OUTPUTS) &&
        (step_increment >= 0.1) &&
        (step_increment <= 100.0)) {
        Lighting_Output[index].Default_Step_Increment = step_increment;
        status = true;
    }

    return status;
}

/**
 * For a given object instance-number, gets the
 * lighting-command-default-priority
 * property value
 *
 * @param object_instance - object-instance number of the object
 *
 * @return the lighting-command-default-priority property value of
 * this object
 */
unsigned Lighting_Output_Default_Priority(
    uint32_t object_instance)
{
    unsigned value = 0;
    unsigned int index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        value = Lighting_Output[index].Lighting_Command_Default_Priority;
    }

    return value;
}

/**
 * For a given object instance-number, sets the
 * lighting-command-default-priority property value of the object.
 *
 * @param object_instance - object-instance number of the object
 * @param priority - holds the value to be set
 *
 * @return true if value was set
 */
bool Lighting_Output_Default_Priority_Set(
    uint32_t object_instance,
    unsigned priority)
{
    bool status = false;
    unsigned int index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if ((index < MAX_LIGHTING_OUTPUTS) &&
        (priority >= BACNET_MIN_PRIORITY) &&
        (priority <= BACNET_MAX_PRIORITY)) {
        Lighting_Output[index].Lighting_Command_Default_Priority = priority;
        status = true;
    }

    return status;
}

/**
 * For a given object instance-number, returns the out-of-service
 * property value
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return  out-of-service property value
 */
bool Lighting_Output_Out_Of_Service(
    uint32_t object_instance)
{
    bool value = false;
    unsigned int index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        value = Lighting_Output[index].Out_Of_Service;
    }

    return value;
}

/**
 * For a given object instance-number, sets the out-of-service property value
 *
 * @param object_instance - object-instance number of the object
 * @param value - boolean out-of-service value
 *
 * @return true if the out-of-service property value was set
 */
void Lighting_Output_Out_Of_Service_Set(
    uint32_t object_instance,
    bool value)
{
    unsigned int index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        Lighting_Output[index].Out_Of_Service = value;
    }
}

/**
 * For a given object instance-number, returns the relinquish-default
 * property value
 *
 * @param  object_instance - object-instance number of the object
 *
 * @return  relinquish-default property value
 */
float Lighting_Output_Relinquish_Default(
    uint32_t object_instance)
{
    float value = 0.0;
    unsigned int index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        value = Lighting_Output[index].Relinquish_Default;
    }

    return value;
}

/**
 * For a given object instance-number, sets the relinquish-default
 * property value
 *
 * @param object_instance - object-instance number of the object
 * @param value - floating point relinquish-default value
 *
 * @return true if the relinquish-default property value was set
 */
bool Lighting_Output_Relinquish_Default_Set(
    uint32_t object_instance,
    float value)
{
    bool status = false;
    unsigned int index = 0;

    index = Lighting_Output_Instance_To_Index(object_instance);
    if (index < MAX_LIGHTING_OUTPUTS) {
        Lighting_Output[index].Relinquish_Default = value;
    }

    return status;
}

/**
 * ReadProperty handler for this object.  For the given ReadProperty
 * data, the application_data is loaded or the error flags are set.
 *
 * @param  rpdata - ReadProperty data, including requested data and
 * data for the reply, or error response.
 *
 * @return number of APDU bytes in the response, or
 * BACNET_STATUS_ERROR on error.
 */
int Lighting_Output_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int len = 0;
    int apdu_len = 0;   /* return value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    BACNET_LIGHTING_COMMAND lighting_command;
    float real_value = (float) 1.414;
    uint32_t unsigned_value = 0;
    unsigned i = 0;
    bool state = false;
    uint8_t *apdu = NULL;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }
    apdu = rpdata->application_data;
    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len =
                encode_application_object_id(&apdu[0], OBJECT_LIGHTING_OUTPUT,
                rpdata->object_instance);
            break;
        case PROP_OBJECT_NAME:
            Lighting_Output_Object_Name(rpdata->object_instance, &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0], OBJECT_LIGHTING_OUTPUT);
            break;
        case PROP_PRESENT_VALUE:
            real_value = Lighting_Output_Present_Value(rpdata->object_instance);
            apdu_len = encode_application_real(&apdu[0], real_value);
            break;
        case PROP_TRACKING_VALUE:
            real_value =
                Lighting_Output_Tracking_Value(rpdata->object_instance);
            apdu_len = encode_application_real(&apdu[0], real_value);
            break;
        case PROP_LIGHTING_COMMAND:
            Lighting_Output_Lighting_Command(
                rpdata->object_instance,
                &lighting_command);
            apdu_len = lighting_command_encode(&apdu[0],
                &lighting_command);
            break;
        case PROP_IN_PROGRESS:
            unsigned_value = Lighting_Output_In_Progress(
                rpdata->object_instance);
            apdu_len = encode_application_enumerated(&apdu[0],
                unsigned_value);
            break;
        case PROP_STATUS_FLAGS:
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            state = Lighting_Output_Out_Of_Service(rpdata->object_instance);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE, state);
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_OUT_OF_SERVICE:
            state = Lighting_Output_Out_Of_Service(rpdata->object_instance);
            apdu_len = encode_application_boolean(&apdu[0], state);
            break;
        case PROP_BLINK_WARN_ENABLE:
            state = Lighting_Output_Blink_Warn_Enable(rpdata->object_instance);
            apdu_len = encode_application_boolean(&apdu[0], state);
            break;
        case PROP_EGRESS_TIME:
            unsigned_value = Lighting_Output_Egress_Time(
                rpdata->object_instance);
            apdu_len = encode_application_unsigned(&apdu[0],
                unsigned_value);
            break;
        case PROP_EGRESS_ACTIVE:
            state = Lighting_Output_Egress_Active(rpdata->object_instance);
            apdu_len = encode_application_boolean(&apdu[0], state);
            break;
        case PROP_DEFAULT_FADE_TIME:
            unsigned_value = Lighting_Output_Default_Fade_Time(
                rpdata->object_instance);
            apdu_len = encode_application_unsigned(&apdu[0],
                unsigned_value);
            break;
        case PROP_DEFAULT_RAMP_RATE:
            real_value =
                Lighting_Output_Default_Ramp_Rate(rpdata->object_instance);
            apdu_len = encode_application_real(&apdu[0], real_value);
            break;
        case PROP_DEFAULT_STEP_INCREMENT:
            real_value =
                Lighting_Output_Default_Step_Increment(rpdata->object_instance);
            apdu_len = encode_application_real(&apdu[0], real_value);
            break;
        case PROP_PRIORITY_ARRAY:
            /* Array element zero is the number of elements in the array */
            if (rpdata->array_index == 0) {
                apdu_len =
                    encode_application_unsigned(&apdu[0], BACNET_MAX_PRIORITY);
            /* if no index was specified, then try to encode the entire list */
            /* into one packet. */
            } else if (rpdata->array_index == BACNET_ARRAY_ALL) {
                for (i = 1; i <= BACNET_MAX_PRIORITY; i++) {
                    if (Lighting_Output_Priority_Active(
                        rpdata->object_instance, i)) {
                        real_value = Lighting_Output_Priority_Value(
                            rpdata->object_instance, i);
                        len =
                            encode_application_real(&apdu[apdu_len],
                            real_value);
                    } else {
                        len = encode_application_null(&apdu[apdu_len]);
                    }
                    /* add it if we have room */
                    if ((apdu_len + len) < MAX_APDU)
                        apdu_len += len;
                    else {
                        rpdata->error_class = ERROR_CLASS_SERVICES;
                        rpdata->error_code = ERROR_CODE_NO_SPACE_FOR_OBJECT;
                        apdu_len = BACNET_STATUS_ERROR;
                        break;
                    }
                }
            } else {
                if (rpdata->array_index <= BACNET_MAX_PRIORITY) {
                    if (Lighting_Output_Priority_Active(
                        rpdata->object_instance,
                        rpdata->array_index)) {
                        real_value = Lighting_Output_Priority_Value(
                            rpdata->object_instance,
                            rpdata->array_index);
                        len =
                            encode_application_real(&apdu[apdu_len],
                            real_value);
                    } else {
                        len = encode_application_null(&apdu[apdu_len]);
                    }
                } else {
                    rpdata->error_class = ERROR_CLASS_PROPERTY;
                    rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                    apdu_len = BACNET_STATUS_ERROR;
                }
            }
            break;
        case PROP_RELINQUISH_DEFAULT:
            real_value = Lighting_Output_Relinquish_Default(
                    rpdata->object_instance);
            apdu_len = encode_application_real(&apdu[0], real_value);
            break;
        case PROP_LIGHTING_COMMAND_DEFAULT_PRIORITY:
            unsigned_value = Lighting_Output_Default_Priority(
                rpdata->object_instance);
            apdu_len = encode_application_unsigned(&apdu[0],
                unsigned_value);
            break;
        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }
    /*  only array properties can have array options */
    if ((apdu_len >= 0) && (rpdata->object_property != PROP_PRIORITY_ARRAY) &&
        (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}

/**
 * WriteProperty handler for this object.  For the given WriteProperty
 * data, the application_data is loaded or the error flags are set.
 *
 * @param  wp_data - BACNET_WRITE_PROPERTY_DATA data, including
 * requested data and space for the reply, or error response.
 *
 * @return false if an error is loaded, true if no errors
 */
bool Lighting_Output_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value */
    int len = 0;
    BACNET_APPLICATION_DATA_VALUE value;

    /* decode the some of the request */
    len =
        bacapp_decode_application_data(wp_data->application_data,
        wp_data->application_data_len, &value);
    /* FIXME: len < application_data_len: more data? */
    if (len < 0) {
        /* error while decoding - a value larger than we can handle */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }
    if ((wp_data->object_property != PROP_PRIORITY_ARRAY) &&
        (wp_data->array_index != BACNET_ARRAY_ALL)) {
        /*  only array properties can have array options */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }
    switch (wp_data->object_property) {
        case PROP_PRESENT_VALUE:
            if (value.tag == BACNET_APPLICATION_TAG_REAL) {
                /* Command priority 6 is reserved for use by Minimum On/Off
                   algorithm and may not be used for other purposes in any
                   object. */
                status =
                    Lighting_Output_Present_Value_Set(wp_data->object_instance,
                    value.type.Real, wp_data->priority);
                if (wp_data->priority == 6) {
                    /* Command priority 6 is reserved for use by Minimum On/Off
                       algorithm and may not be used for other purposes in any
                       object. */
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                } else if (!status) {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
                status =
                    WPValidateArgType(&value, BACNET_APPLICATION_TAG_NULL,
                    &wp_data->error_class, &wp_data->error_code);
                if (status) {
                    if (wp_data->priority == 6) {
                        /* Command priority 6 is reserved for use by Minimum On/Off
                           algorithm and may not be used for other purposes in any
                           object.  - Note Lighting_Output_Present_Value_Relinquish()
                           will have returned false because of this */
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                    } else {
                        status = Lighting_Output_Present_Value_Relinquish(
                            wp_data->object_instance, wp_data->priority);
                        if (!status) {
                            wp_data->error_class = ERROR_CLASS_PROPERTY;
                            wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                        }
                    }
                }
            }
            break;
        case PROP_LIGHTING_COMMAND:
            if (value.tag == BACNET_APPLICATION_TAG_LIGHTING_COMMAND) {
                status = Lighting_Output_Lighting_Command_Set(
                    wp_data->object_instance,
                    &value.type.Lighting_Command);
                if (!status) {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
            }
            break;
        case PROP_OUT_OF_SERVICE:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_BOOLEAN,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                Lighting_Output_Out_Of_Service_Set(
                    wp_data->object_instance,
                    value.type.Boolean);
            }
            break;
        case PROP_OBJECT_IDENTIFIER:
        case PROP_OBJECT_NAME:
        case PROP_OBJECT_TYPE:
        case PROP_TRACKING_VALUE:
        case PROP_IN_PROGRESS:
        case PROP_STATUS_FLAGS:
        case PROP_BLINK_WARN_ENABLE:
        case PROP_EGRESS_TIME:
        case PROP_EGRESS_ACTIVE:
        case PROP_DEFAULT_FADE_TIME:
        case PROP_DEFAULT_RAMP_RATE:
        case PROP_DEFAULT_STEP_INCREMENT:
        case PROP_PRIORITY_ARRAY:
        case PROP_RELINQUISH_DEFAULT:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            break;
        default:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            break;
    }

    return status;
}

/**
 * Handles the timing for a single Lighting Output object Ramp
 *
 * @param pLight - Lighting Output object
 * @param pCommand - BACNET_LIGHTING_COMMAND of the Lighting Output object
 * @param milliseconds - number of milliseconds elapsed since previously
 * called.  Works best when called about every 10 milliseconds.
 */
static void Lighting_Output_Ramp_Handler(
    struct lighting_output_object *pLight,
    BACNET_LIGHTING_COMMAND *pCommand,
    uint16_t milliseconds)
{
    if (pLight && pCommand) {

    }
}

/**
 * Handles the timing for a single Lighting Output object Fade
 *
 * @param pLight - Lighting Output object
 * @param pCommand - BACNET_LIGHTING_COMMAND of the Lighting Output object
 * @param milliseconds - number of milliseconds elapsed since previously
 * called.  Works best when called about every 10 milliseconds.
 */
static void Lighting_Output_Fade_Handler(
    struct lighting_output_object *pLight,
    BACNET_LIGHTING_COMMAND *pCommand,
    uint16_t milliseconds)
{
    if (pLight && pCommand) {

    }
}

/**
 * Handles the timing for a single Lighting Output object
 *
 * @param index - 0..MAX_LIGHTING_OUTPUTS value
 * @param milliseconds - number of milliseconds elapsed since previously
 * called.  Works best when called about every 10 milliseconds.
 */
static void Lighting_Output_Timer_Handler(
    unsigned index,
    uint16_t milliseconds)
{
    struct lighting_output_object *pLight = NULL;
    BACNET_LIGHTING_COMMAND *pCommand = NULL;

    if (index < MAX_LIGHTING_OUTPUTS) {
        pLight = &Lighting_Output[index];
        pCommand = &pLight->Lighting_Command;
        switch (pCommand->operation) {
            case BACNET_LIGHTS_NONE:
                break;
            case BACNET_LIGHTS_FADE_TO:
                Lighting_Output_Fade_Handler(pLight, pCommand, milliseconds);
                break;
            case BACNET_LIGHTS_RAMP_TO:
                Lighting_Output_Ramp_Handler(pLight, pCommand, milliseconds);
                break;
            case BACNET_LIGHTS_STEP_UP:
                break;
            case BACNET_LIGHTS_STEP_DOWN:
                break;
            case BACNET_LIGHTS_STEP_ON:
                break;
            case BACNET_LIGHTS_STEP_OFF:
                break;
            case BACNET_LIGHTS_WARN:
                break;
            case BACNET_LIGHTS_WARN_OFF:
                break;
            case BACNET_LIGHTS_WARN_RELINQUISH:
                break;
            case BACNET_LIGHTS_STOP:
                break;
            default:
                break;
        }
    }
}

/**
 * Initializes the Lighting Output object data
 *
 * @param milliseconds - number of milliseconds elapsed since previously
 * called.  Works best when called about every 10 milliseconds.
 */
void Lighting_Output_Timer(
    uint16_t milliseconds)
{
    unsigned i = 0;

    for (i = 0; i < MAX_LIGHTING_OUTPUTS; i++) {
        Lighting_Output_Timer_Handler(i, milliseconds);
    }
}

/**
 * Initializes the Lighting Output object data
 */
void Lighting_Output_Init(
    void)
{
    unsigned i, p;

    for (i = 0; i < MAX_LIGHTING_OUTPUTS; i++) {
        Lighting_Output[i].Present_Value = 0.0;
        Lighting_Output[i].Tracking_Value = 0.0;
        Lighting_Output[i].Physical_Value = 0.0;
        Lighting_Output[i].Lighting_Command.operation = BACNET_LIGHTS_NONE;
        Lighting_Output[i].Lighting_Command.use_target_level = false;
        Lighting_Output[i].Lighting_Command.use_ramp_rate = false;
        Lighting_Output[i].Lighting_Command.use_step_increment = false;
        Lighting_Output[i].Lighting_Command.use_fade_time = false;
        Lighting_Output[i].Lighting_Command.use_priority = false;
        Lighting_Output[i].In_Progress = BACNET_LIGHTING_IDLE;
        Lighting_Output[i].Out_Of_Service = false;
        Lighting_Output[i].Blink_Warn_Enable = false;
        Lighting_Output[i].Egress_Active = false;
        Lighting_Output[i].Egress_Time = 0;
        Lighting_Output[i].Default_Fade_Time = 100;
        Lighting_Output[i].Default_Ramp_Rate = 100.0;
        Lighting_Output[i].Default_Step_Increment = 1.0;
        Lighting_Output[i].Transition = BACNET_LIGHTING_TRANSITION_IDLE;
        Lighting_Output[i].Feedback_Value = 0.0;
        for (p = 0; p < BACNET_MAX_PRIORITY; p++) {
            Lighting_Output[i].Priority_Array[p] = 0.0;
            BIT_CLEAR(Lighting_Output[i].Priority_Active_Bits, p);
        }
        Lighting_Output[i].Relinquish_Default = 0.0;
        Lighting_Output[i].Power = 0.0;
        Lighting_Output[i].Instantaneous_Power = 0.0;
        Lighting_Output[i].Min_Actual_Value = 0.0;
        Lighting_Output[i].Max_Actual_Value = 100.0;
        Lighting_Output[i].Lighting_Command_Default_Priority = 16;
    }

    return;
}

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

bool WPValidateArgType(
    BACNET_APPLICATION_DATA_VALUE * pValue,
    uint8_t ucExpectedTag,
    BACNET_ERROR_CLASS * pErrorClass,
    BACNET_ERROR_CODE * pErrorCode)
{
    pValue = pValue;
    ucExpectedTag = ucExpectedTag;
    pErrorClass = pErrorClass;
    pErrorCode = pErrorCode;

    return false;
}

void testLightingOutput(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0;
    uint32_t len_value = 0;
    uint8_t tag_number = 0;
    uint16_t decoded_type = 0;
    uint32_t decoded_instance = 0;
    BACNET_READ_PROPERTY_DATA rpdata;

    Lighting_Output_Init();
    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = OBJECT_LIGHTING_OUTPUT;
    rpdata.object_instance = 1;
    rpdata.object_property = PROP_OBJECT_IDENTIFIER;
    rpdata.array_index = BACNET_ARRAY_ALL;
    len = Lighting_Output_Read_Property(&rpdata);
    ct_test(pTest, len != 0);
    len = decode_tag_number_and_value(&apdu[0], &tag_number, &len_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_OBJECT_ID);
    len = decode_object_id(&apdu[len], &decoded_type, &decoded_instance);
    ct_test(pTest, decoded_type == rpdata.object_type);
    ct_test(pTest, decoded_instance == rpdata.object_instance);

    return;
}

#ifdef TEST_LIGHTING_OUTPUT
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Lighting Output", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testLightingOutput);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_LIGHTING_INPUT */
#endif /* TEST */
