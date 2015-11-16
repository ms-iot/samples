/**
 * @file
 * @author Steve Karg
 * @date 2013
 * @brief Channel objects, customize for your use
 *
 * @section DESCRIPTION
 *
 * The Channel object is a command object without a priority array, and the
 * present-value property uses a priority array and a single precision floating point
 * data type.
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
 */
#ifndef CHANNEL_H
#define CHANNEL_H

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "rp.h"
#include "wp.h"
#include "lo.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* BACNET_CHANNEL_VALUE decodes WriteProperty service requests
   Choose the datatypes that your application supports */
#if !(defined(CHANNEL_NUMERIC) || \
    defined(CHANNEL_NULL) || \
    defined(CHANNEL_BOOLEAN) || \
    defined(CHANNEL_UNSIGNED) || \
    defined(CHANNEL_SIGNED) || \
    defined(CHANNEL_REAL) || \
    defined(CHANNEL_DOUBLE) || \
    defined(CHANNEL_OCTET_STRING) || \
    defined(CHANNEL_CHARACTER_STRING) || \
    defined(CHANNEL_BIT_STRING) || \
    defined(CHANNEL_ENUMERATED) || \
    defined(CHANNEL_DATE) || \
    defined(CHANNEL_TIME) || \
    defined(CHANNEL_OBJECT_ID) || \
    defined(CHANNEL_LIGHTING_COMMAND))
#define CHANNEL_NUMERIC
#endif

#if defined (CHANNEL_NUMERIC)
#define CHANNEL_NULL
#define CHANNEL_BOOLEAN
#define CHANNEL_UNSIGNED
#define CHANNEL_SIGNED
#define CHANNEL_REAL
#define CHANNEL_DOUBLE
#define CHANNEL_ENUMERATED
#define CHANNEL_LIGHTING_COMMAND
#endif

    typedef struct BACnet_Channel_Value_t {
        uint8_t tag;
        union {
            /* NULL - not needed as it is encoded in the tag alone */
#if defined (CHANNEL_BOOLEAN)
            bool Boolean;
#endif
#if defined (CHANNEL_UNSIGNED)
            uint32_t Unsigned_Int;
#endif
#if defined (CHANNEL_SIGNED)
            int32_t Signed_Int;
#endif
#if defined (CHANNEL_REAL)
            float Real;
#endif
#if defined (CHANNEL_DOUBLE)
            double Double;
#endif
#if defined (CHANNEL_OCTET_STRING)
            BACNET_OCTET_STRING Octet_String;
#endif
#if defined (CHANNEL_CHARACTER_STRING)
            BACNET_CHARACTER_STRING Character_String;
#endif
#if defined (CHANNEL_BIT_STRING)
            BACNET_BIT_STRING Bit_String;
#endif
#if defined (CHANNEL_ENUMERATED)
            uint32_t Enumerated;
#endif
#if defined (CHANNEL_DATE)
            BACNET_DATE Date;
#endif
#if defined (CHANNEL_TIME)
            BACNET_TIME Time;
#endif
#if defined (CHANNEL_OBJECT_ID)
            BACNET_OBJECT_ID Object_Id;
#endif
#if defined (CHANNEL_LIGHTING_COMMAND)
            BACNET_LIGHTING_COMMAND Lighting_Command;
#endif
        } type;
        /* simple linked list if needed */
        struct BACnet_Channel_Value_t *next;
    } BACNET_CHANNEL_VALUE;

    void Channel_Property_Lists(const int **pRequired,
        const int **pOptional,
        const int **pProprietary);
    bool Channel_Valid_Instance(uint32_t object_instance);
    unsigned Channel_Count(void);
    uint32_t Channel_Index_To_Instance(unsigned index);
    unsigned Channel_Instance_To_Index(uint32_t instance);
    bool Channel_Object_Instance_Add(uint32_t instance);

    bool Channel_Object_Name(uint32_t object_instance,
        BACNET_CHARACTER_STRING * object_name);
    bool Channel_Name_Set(uint32_t object_instance,
        char *new_name);

    int Channel_Read_Property(BACNET_READ_PROPERTY_DATA * rpdata);
    bool Channel_Write_Property(BACNET_WRITE_PROPERTY_DATA * wp_data);

    BACNET_CHANNEL_VALUE * Channel_Present_Value(uint32_t object_instance);
    bool Channel_Present_Value_Set(
        BACNET_WRITE_PROPERTY_DATA * wp_data,
        BACNET_APPLICATION_DATA_VALUE * value);

    bool Channel_Out_Of_Service(uint32_t object_instance);
    void Channel_Out_Of_Service_Set(uint32_t object_instance,
        bool oos_flag);

    unsigned Channel_Last_Priority(uint32_t object_instance);
    BACNET_WRITE_STATUS Channel_Write_Status(uint32_t object_instance);
    uint16_t Channel_Number(uint32_t object_instance);
    bool Channel_Number_Set(uint32_t object_instance, uint16_t value);

    unsigned Channel_Reference_List_Member_Count(uint32_t object_instance);
    BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE *
    Channel_Reference_List_Member_Element(uint32_t object_instance,
        unsigned element);
    bool Channel_Reference_List_Member_Element_Set(uint32_t object_instance,
        unsigned array_index,
        BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE *pMemberSrc);
    unsigned Channel_Reference_List_Member_Element_Add(uint32_t object_instance,
        BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE *pMemberSrc);
    unsigned Channel_Reference_List_Member_Local_Add(
        uint32_t object_instance,
        uint16_t type,
        uint32_t instance,
        BACNET_PROPERTY_ID propertyIdentifier,
        uint32_t arrayIndex);
    uint16_t Channel_Control_Groups_Element(
        uint32_t object_instance,
        int32_t array_index);
    bool Channel_Control_Groups_Element_Set(
        uint32_t object_instance,
        int32_t array_index,
        uint16_t value);
    bool Channel_Value_Copy(BACNET_CHANNEL_VALUE * cvalue,
        BACNET_APPLICATION_DATA_VALUE * value);
    int Channel_Value_Encode(uint8_t *apdu, int apdu_max,
        BACNET_CHANNEL_VALUE * value);
    int Channel_Coerce_Data_Encode(
        uint8_t * apdu,
        unsigned max_apdu,
        BACNET_APPLICATION_DATA_VALUE * value,
        BACNET_APPLICATION_TAG tag);
    bool Channel_Write_Member_Value(
        BACNET_WRITE_PROPERTY_DATA * wp_data,
        BACNET_APPLICATION_DATA_VALUE * value);

    void Channel_Init(void);

#ifdef TEST
#include "ctest.h"
    void testChannelObject(Test * pTest);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
