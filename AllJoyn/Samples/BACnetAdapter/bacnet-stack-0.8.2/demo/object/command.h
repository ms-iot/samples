/**
 * @file
 * @author Nikola Jelic
 * @date 2014
 * @brief Command objects, customize for your use
 *
 * @section DESCRIPTION
 *
 * The Command object type defines a standardized object whose
 * properties represent the externally visible characteristics of a
 * multi-action command procedure. A Command object is used to
 * write a set of values to a group of object properties, based on
 * the "action code" that is written to the Present_Value of the
 * Command object. Whenever the Present_Value property of the
 * Command object is written to, it triggers the Command object
 * to take a set of actions that change the values of a set of other
 * objects' properties.
 *
 * @section LICENSE
 *
 * Copyright (C) 2014 Nikola Jelic <nikola.jelic@euroicc.com>
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
#ifndef COMMAND_H
#define COMMAND_H

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "rp.h"
#include "wp.h"

#ifndef MAX_COMMANDS
#define MAX_COMMANDS 4
#endif

#ifndef MAX_COMMAND_ACTIONS
#define MAX_COMMAND_ACTIONS 8
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    typedef struct bacnet_action_list {
        BACNET_OBJECT_ID Device_Id;     /* Optional */
        BACNET_OBJECT_ID Object_Id;
        BACNET_PROPERTY_ID Property_Identifier;
        uint32_t Property_Array_Index;  /* Conditional */
        BACNET_APPLICATION_DATA_VALUE Value;
        uint8_t Priority;       /* Conditional */
        uint32_t Post_Delay;    /* Optional */
        bool Quit_On_Failure;
        bool Write_Successful;
        struct bacnet_action_list *next;
    } BACNET_ACTION_LIST;

    int cl_encode_apdu(
        uint8_t * apdu,
        BACNET_ACTION_LIST * bcl);

    int cl_decode_apdu(
        uint8_t * apdu,
        unsigned apdu_len,
        BACNET_APPLICATION_TAG tag,
        BACNET_ACTION_LIST * bcl);

    typedef struct command_descr {
        uint32_t Present_Value;
        bool In_Process;
        bool All_Writes_Successful;
        BACNET_ACTION_LIST Action[MAX_COMMAND_ACTIONS];
    } COMMAND_DESCR;

    void Command_Property_Lists(
        const int **pRequired,
        const int **pOptional,
        const int **pProprietary);

    bool Command_Valid_Instance(
        uint32_t object_instance);
    unsigned Command_Count(
        void);
    uint32_t Command_Index_To_Instance(
        unsigned index);
    unsigned Command_Instance_To_Index(
        uint32_t instance);
    bool Command_Object_Instance_Add(
        uint32_t instance);

    bool Command_Object_Name(
        uint32_t object_instance,
        BACNET_CHARACTER_STRING * object_name);
    bool Command_Name_Set(
        uint32_t object_instance,
        char *new_name);

    char *Command_Description(
        uint32_t instance);
    bool Command_Description_Set(
        uint32_t instance,
        char *new_name);

    int Command_Read_Property(
        BACNET_READ_PROPERTY_DATA * rpdata);
    bool Command_Write_Property(
        BACNET_WRITE_PROPERTY_DATA * wp_data);

    uint32_t Command_Present_Value(
        uint32_t object_instance);
    bool Command_Present_Value_Set(
        uint32_t object_instance,
        uint32_t value);

    bool Command_In_Process(
        uint32_t object_instance);
    bool Command_In_Process_Set(
        uint32_t object_instance,
        bool value);

    bool Command_All_Writes_Successful(
        uint32_t object_instance);
    bool Command_All_Writes_Successful_Set(
        uint32_t object_instance,
        bool value);

    bool Command_Change_Of_Value(
        uint32_t instance);
    void Command_Change_Of_Value_Clear(
        uint32_t instance);
    bool Command_Encode_Value_List(
        uint32_t object_instance,
        BACNET_PROPERTY_VALUE * value_list);
    float Command_COV_Increment(
        uint32_t instance);
    void Command_COV_Increment_Set(
        uint32_t instance,
        float value);

    /* note: header of Intrinsic_Reporting function is required
       even when INTRINSIC_REPORTING is not defined */
    void Command_Intrinsic_Reporting(
        uint32_t object_instance);

    void Command_Init(
        void);

#ifdef TEST
#include "ctest.h"
    void testCommand(
        Test * pTest);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
