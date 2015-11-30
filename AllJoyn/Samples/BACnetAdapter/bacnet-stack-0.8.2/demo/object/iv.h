/**
 * @file
 * @author Steve Karg
 * @date 2014
 * @brief Integer Value objects, customize for your use
 *
 * @section DESCRIPTION
 *
 * The Integer Value object is an object with a present-value that
 * uses an INTEGER data type.
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
#ifndef IV_H
#define IV_H

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "bacerror.h"
#include "wp.h"
#include "rp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    void Integer_Value_Property_Lists(
        const int **pRequired,
        const int **pOptional,
        const int **pProprietary);
    bool Integer_Value_Valid_Instance(
        uint32_t object_instance);
    unsigned Integer_Value_Count(
        void);
    uint32_t Integer_Value_Index_To_Instance(
        unsigned index);
    unsigned Integer_Value_Instance_To_Index(
        uint32_t object_instance);

    bool Integer_Value_Object_Name(
        uint32_t object_instance,
        BACNET_CHARACTER_STRING * object_name);

    int Integer_Value_Read_Property(
        BACNET_READ_PROPERTY_DATA * rpdata);

    bool Integer_Value_Write_Property(
        BACNET_WRITE_PROPERTY_DATA * wp_data);

    bool Integer_Value_Present_Value_Set(
        uint32_t object_instance,
        int32_t value,
        uint8_t priority);
    int32_t Integer_Value_Present_Value(
        uint32_t object_instance);

    bool Integer_Value_Change_Of_Value(
        uint32_t instance);
    void Integer_Value_Change_Of_Value_Clear(
        uint32_t instance);
    bool Integer_Value_Encode_Value_List(
        uint32_t object_instance,
        BACNET_PROPERTY_VALUE * value_list);
    float Integer_Value_COV_Increment(
        uint32_t instance);
    void Integer_Value_COV_Increment_Set(
        uint32_t instance,
        float value);

    char *Integer_Value_Description(
        uint32_t instance);
    bool Integer_Value_Description_Set(
        uint32_t instance,
        char *new_name);

    uint16_t Integer_Value_Units(
        uint32_t instance);
    bool Integer_Value_Units_Set(
        uint32_t instance,
        uint16_t unit);

    bool Integer_Value_Out_Of_Service(
        uint32_t instance);
    void Integer_Value_Out_Of_Service_Set(
        uint32_t instance,
        bool oos_flag);

    void Integer_Value_Init(
        void);

#ifdef TEST
#include "ctest.h"
    void testInteger_Value(
        Test * pTest);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
