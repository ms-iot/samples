/**************************************************************************
*
* Copyright (C) 2014 Steve Karg <skarg@users.sourceforge.net>
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
#include <stdbool.h>
#include <stdint.h>
#include <string.h>     /* for memmove */
#include <time.h>       /* for timezone, localtime */
#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "bacapp.h"
#include "config.h"     /* the custom stuff */
#include "apdu.h"
#include "wp.h" /* WriteProperty handling */
#include "rp.h" /* ReadProperty handling */
#include "dcc.h"        /* DeviceCommunicationControl handling */
#include "version.h"
#include "device.h"     /* me */
#include "handlers.h"
#include "datalink.h"
#include "address.h"
/* include the OS specific */
#include "timer.h"
/* include the device object */
#include "device.h"
#include "bi.h"
#include "bo.h"

/* local forward (semi-private) and external prototypes */
int Device_Read_Property_Local(
    BACNET_READ_PROPERTY_DATA * rpdata);
bool Device_Write_Property_Local(
    BACNET_WRITE_PROPERTY_DATA * wp_data);
extern int Routed_Device_Read_Property_Local(
    BACNET_READ_PROPERTY_DATA * rpdata);
extern bool Routed_Device_Write_Property_Local(
    BACNET_WRITE_PROPERTY_DATA * wp_data);

/* may be overridden by outside table */
static object_functions_t *Object_Table;

static object_functions_t My_Object_Table[] = {
    {OBJECT_DEVICE,
            NULL /* Init - don't init Device or it will recourse! */ ,
            Device_Count,
            Device_Index_To_Instance,
            Device_Valid_Object_Instance_Number,
            Device_Object_Name,
            Device_Read_Property_Local,
            Device_Write_Property_Local,
            Device_Property_Lists,
            DeviceGetRRInfo,
            NULL /* Iterator */ ,
            NULL /* Value_Lists */ ,
            NULL /* COV */ ,
            NULL /* COV Clear */ ,
        NULL /* Intrinsic Reporting */ },
    {OBJECT_BINARY_INPUT,
            Binary_Input_Init,
            Binary_Input_Count,
            Binary_Input_Index_To_Instance,
            Binary_Input_Valid_Instance,
            Binary_Input_Object_Name,
            Binary_Input_Read_Property,
            Binary_Input_Write_Property,
            Binary_Input_Property_Lists,
            NULL /* ReadRangeInfo */ ,
            NULL /* Iterator */ ,
            Binary_Input_Encode_Value_List,
            Binary_Input_Change_Of_Value,
            Binary_Input_Change_Of_Value_Clear,
        NULL /* Intrinsic Reporting */ },
    {OBJECT_BINARY_OUTPUT,
            Binary_Output_Init,
            Binary_Output_Count,
            Binary_Output_Index_To_Instance,
            Binary_Output_Valid_Instance,
            Binary_Output_Object_Name,
            Binary_Output_Read_Property,
            Binary_Output_Write_Property,
            Binary_Output_Property_Lists,
            NULL /* ReadRangeInfo */ ,
            NULL /* Iterator */ ,
            NULL /* Value_Lists */ ,
            NULL /* COV */ ,
            NULL /* COV Clear */ ,
        NULL /* Intrinsic Reporting */ },
    {MAX_BACNET_OBJECT_TYPE,
            NULL /* Init */ ,
            NULL /* Count */ ,
            NULL /* Index_To_Instance */ ,
            NULL /* Valid_Instance */ ,
            NULL /* Object_Name */ ,
            NULL /* Read_Property */ ,
            NULL /* Write_Property */ ,
            NULL /* Property_Lists */ ,
            NULL /* ReadRangeInfo */ ,
            NULL /* Iterator */ ,
            NULL /* Value_Lists */ ,
            NULL /* COV */ ,
            NULL /* COV Clear */ ,
        NULL /* Intrinsic Reporting */ }
};

/** Glue function to let the Device object, when called by a handler,
 * lookup which Object type needs to be invoked.
 * @ingroup ObjHelpers
 * @param Object_Type [in] The type of BACnet Object the handler wants to access.
 * @return Pointer to the group of object helper functions that implement this
 *         type of Object.
 */
static struct object_functions *Device_Objects_Find_Functions(
    BACNET_OBJECT_TYPE Object_Type)
{
    struct object_functions *pObject = NULL;

    pObject = Object_Table;
    while (pObject->Object_Type < MAX_BACNET_OBJECT_TYPE) {
        /* handle each object type */
        if (pObject->Object_Type == Object_Type) {
            return (pObject);
        }
        pObject++;
    }

    return (NULL);
}

/** Try to find a rr_info_function helper function for the requested object type.
 * @ingroup ObjIntf
 *
 * @param object_type [in] The type of BACnet Object the handler wants to access.
 * @return Pointer to the object helper function that implements the
 *         ReadRangeInfo function, Object_RR_Info, for this type of Object on
 *         success, else a NULL pointer if the type of Object isn't supported
 *         or doesn't have a ReadRangeInfo function.
 */
rr_info_function Device_Objects_RR_Info(
    BACNET_OBJECT_TYPE object_type)
{
    struct object_functions *pObject = NULL;

    pObject = Device_Objects_Find_Functions(object_type);
    return (pObject != NULL ? pObject->Object_RR_Info : NULL);
}

/** For a given object type, returns the special property list.
 * This function is used for ReadPropertyMultiple calls which want
 * just Required, just Optional, or All properties.
 * @ingroup ObjIntf
 *
 * @param object_type [in] The desired BACNET_OBJECT_TYPE whose properties
 *            are to be listed.
 * @param pPropertyList [out] Reference to the structure which will, on return,
 *            list, separately, the Required, Optional, and Proprietary object
 *            properties with their counts.
 */
void Device_Objects_Property_List(
    BACNET_OBJECT_TYPE object_type,
    struct special_property_list_t *pPropertyList)
{
    struct object_functions *pObject = NULL;

    pPropertyList->Required.pList = NULL;
    pPropertyList->Optional.pList = NULL;
    pPropertyList->Proprietary.pList = NULL;

    /* If we can find an entry for the required object type
     * and there is an Object_List_RPM fn ptr then call it
     * to populate the pointers to the individual list counters.
     */

    pObject = Device_Objects_Find_Functions(object_type);
    if ((pObject != NULL) && (pObject->Object_RPM_List != NULL)) {
        pObject->Object_RPM_List(&pPropertyList->Required.pList,
            &pPropertyList->Optional.pList, &pPropertyList->Proprietary.pList);
    }

    /* Fetch the counts if available otherwise zero them */
    pPropertyList->Required.count =
        pPropertyList->Required.pList ==
        NULL ? 0 : property_list_count(pPropertyList->Required.pList);

    pPropertyList->Optional.count =
        pPropertyList->Optional.pList ==
        NULL ? 0 : property_list_count(pPropertyList->Optional.pList);

    pPropertyList->Proprietary.count =
        pPropertyList->Proprietary.pList ==
        NULL ? 0 : property_list_count(pPropertyList->Proprietary.pList);

    return;
}

/** Commands a Device re-initialization, to a given state.
 * The request's password must match for the operation to succeed.
 * This implementation provides a framework, but doesn't
 * actually *DO* anything.
 * @note You could use a mix of states and passwords to multiple outcomes.
 * @note You probably want to restart *after* the simple ack has been sent
 *       from the return handler, so just set a local flag here.
 * @ingroup ObjIntf
 *
 * @param rd_data [in,out] The information from the RD request.
 *                         On failure, the error class and code will be set.
 * @return True if succeeds (password is correct), else False.
 */
bool Device_Reinitialize(
    BACNET_REINITIALIZE_DEVICE_DATA * rd_data)
{
    bool status = false;

    if (characterstring_ansi_same(&rd_data->password, "raspberry")) {
        switch (rd_data->state) {
            case BACNET_REINIT_COLDSTART:
            case BACNET_REINIT_WARMSTART:
                dcc_set_status_duration(COMMUNICATION_ENABLE, 0);
                break;
            case BACNET_REINIT_STARTBACKUP:
                break;
            case BACNET_REINIT_ENDBACKUP:
                break;
            case BACNET_REINIT_STARTRESTORE:
                break;
            case BACNET_REINIT_ENDRESTORE:
                break;
            case BACNET_REINIT_ABORTRESTORE:
                break;
            default:
                break;
        }
        /* Note: you could use a mix of state
           and password to multiple things */
        /* note: you probably want to restart *after* the
           simple ack has been sent from the return handler
           so just set a flag from here */
        status = true;
    } else {
        rd_data->error_class = ERROR_CLASS_SECURITY;
        rd_data->error_code = ERROR_CODE_PASSWORD_FAILURE;
    }

    return status;
}

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Device_Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_SYSTEM_STATUS,
    PROP_VENDOR_NAME,
    PROP_VENDOR_IDENTIFIER,
    PROP_MODEL_NAME,
    PROP_FIRMWARE_REVISION,
    PROP_APPLICATION_SOFTWARE_VERSION,
    PROP_PROTOCOL_VERSION,
    PROP_PROTOCOL_REVISION,
    PROP_PROTOCOL_SERVICES_SUPPORTED,
    PROP_PROTOCOL_OBJECT_TYPES_SUPPORTED,
    PROP_OBJECT_LIST,
    PROP_MAX_APDU_LENGTH_ACCEPTED,
    PROP_SEGMENTATION_SUPPORTED,
    PROP_APDU_TIMEOUT,
    PROP_NUMBER_OF_APDU_RETRIES,
    PROP_DEVICE_ADDRESS_BINDING,
    PROP_DATABASE_REVISION,
    -1
};

static const int Device_Properties_Optional[] = {
#if defined(BACDL_MSTP)
    PROP_MAX_MASTER,
    PROP_MAX_INFO_FRAMES,
#endif
    PROP_DESCRIPTION,
    PROP_LOCAL_TIME,
    PROP_UTC_OFFSET,
    PROP_LOCAL_DATE,
    PROP_DAYLIGHT_SAVINGS_STATUS,
    PROP_LOCATION,
    PROP_ACTIVE_COV_SUBSCRIPTIONS,
    -1
};

static const int Device_Properties_Proprietary[] = {
    -1
};

void Device_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
    if (pRequired)
        *pRequired = Device_Properties_Required;
    if (pOptional)
        *pOptional = Device_Properties_Optional;
    if (pProprietary)
        *pProprietary = Device_Properties_Proprietary;

    return;
}

/* note: you really only need to define variables for
   properties that are writable or that may change.
   The properties that are constant can be hard coded
   into the read-property encoding. */

static uint32_t Object_Instance_Number = 260001;
static BACNET_CHARACTER_STRING My_Object_Name;
static BACNET_DEVICE_STATUS System_Status = STATUS_OPERATIONAL;
static char *Vendor_Name = BACNET_VENDOR_NAME;
static uint16_t Vendor_Identifier = BACNET_VENDOR_ID;
static char Model_Name[MAX_DEV_MOD_LEN + 1] = "PiFace Digital";
static char Application_Software_Version[MAX_DEV_VER_LEN + 1] = "1.0";
static char Location[MAX_DEV_LOC_LEN + 1] = "USA";
static char Description[MAX_DEV_DESC_LEN + 1] = "Raspberry PiFace Digital Demo";
/* static uint8_t Protocol_Version = 1; - constant, not settable */
/* static uint8_t Protocol_Revision = 4; - constant, not settable */
/* Protocol_Services_Supported - dynamically generated */
/* Protocol_Object_Types_Supported - in RP encoding */
/* Object_List - dynamically generated */
/* static BACNET_SEGMENTATION Segmentation_Supported = SEGMENTATION_NONE; */
/* static uint8_t Max_Segments_Accepted = 0; */
/* VT_Classes_Supported */
/* Active_VT_Sessions */
static BACNET_TIME Local_Time;  /* rely on OS, if there is one */
static BACNET_DATE Local_Date;  /* rely on OS, if there is one */
/* NOTE: BACnet UTC Offset is inverse of common practice.
   If your UTC offset is -5hours of GMT,
   then BACnet UTC offset is +5hours.
   BACnet UTC offset is expressed in minutes. */
static int32_t UTC_Offset = 5 * 60;
static bool Daylight_Savings_Status = false;    /* rely on OS */
/* List_Of_Session_Keys */
/* Time_Synchronization_Recipients */
/* Max_Master - rely on MS/TP subsystem, if there is one */
/* Max_Info_Frames - rely on MS/TP subsystem, if there is one */
/* Device_Address_Binding - required, but relies on binding cache */
static uint32_t Database_Revision = 0;
/* Configuration_Files */
/* Last_Restore_Time */
/* Backup_Failure_Timeout */
/* Active_COV_Subscriptions */
/* Slave_Proxy_Enable */
/* Manual_Slave_Address_Binding */
/* Auto_Slave_Discovery */
/* Slave_Address_Binding */
/* Profile_Name */

unsigned Device_Count(
    void)
{
    return 1;
}

uint32_t Device_Index_To_Instance(
    unsigned index)
{
    index = index;
    return Object_Instance_Number;
}

/* methods to manipulate the data */

/** Return the Object Instance number for our (single) Device Object.
 * This is a key function, widely invoked by the handler code, since
 * it provides "our" (ie, local) address.
 * @ingroup ObjIntf
 * @return The Instance number used in the BACNET_OBJECT_ID for the Device.
 */
uint32_t Device_Object_Instance_Number(
    void)
{
#ifdef BAC_ROUTING
    return Routed_Device_Object_Instance_Number();
#else
    return Object_Instance_Number;
#endif
}

bool Device_Set_Object_Instance_Number(
    uint32_t object_id)
{
    bool status = true; /* return value */

    if (object_id <= BACNET_MAX_INSTANCE) {
        /* Make the change and update the database revision */
        Object_Instance_Number = object_id;
        Device_Inc_Database_Revision();
    } else
        status = false;

    return status;
}

bool Device_Valid_Object_Instance_Number(
    uint32_t object_id)
{
    return (Object_Instance_Number == object_id);
}

bool Device_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    bool status = false;

    if (object_instance == Object_Instance_Number) {
        status = characterstring_copy(object_name, &My_Object_Name);
    }

    return status;
}

bool Device_Set_Object_Name(
    BACNET_CHARACTER_STRING * object_name)
{
    bool status = false;        /*return value */

    if (!characterstring_same(&My_Object_Name, object_name)) {
        /* Make the change and update the database revision */
        status = characterstring_copy(&My_Object_Name, object_name);
        Device_Inc_Database_Revision();
    }

    return status;
}

BACNET_DEVICE_STATUS Device_System_Status(
    void)
{
    return System_Status;
}

int Device_Set_System_Status(
    BACNET_DEVICE_STATUS status,
    bool local)
{
    int result = 0;     /*return value - 0 = ok, -1 = bad value, -2 = not allowed */

    /* We limit the options available depending on whether the source is
     * internal or external. */
    if (local) {
        switch (status) {
            case STATUS_OPERATIONAL:
            case STATUS_OPERATIONAL_READ_ONLY:
            case STATUS_DOWNLOAD_REQUIRED:
            case STATUS_DOWNLOAD_IN_PROGRESS:
            case STATUS_NON_OPERATIONAL:
                System_Status = status;
                break;

                /* Don't support backup at present so don't allow setting */
            case STATUS_BACKUP_IN_PROGRESS:
                result = -2;
                break;

            default:
                result = -1;
                break;
        }
    } else {
        switch (status) {
                /* Allow these for the moment as a way to easily alter
                 * overall device operation. The lack of password protection
                 * or other authentication makes allowing writes to this
                 * property a risky facility to provide.
                 */
            case STATUS_OPERATIONAL:
            case STATUS_OPERATIONAL_READ_ONLY:
            case STATUS_NON_OPERATIONAL:
                System_Status = status;
                break;

                /* Don't allow outsider set this - it should probably
                 * be set if the device config is incomplete or
                 * corrupted or perhaps after some sort of operator
                 * wipe operation.
                 */
            case STATUS_DOWNLOAD_REQUIRED:
                /* Don't allow outsider set this - it should be set
                 * internally at the start of a multi packet download
                 * perhaps indirectly via PT or WF to a config file.
                 */
            case STATUS_DOWNLOAD_IN_PROGRESS:
                /* Don't support backup at present so don't allow setting */
            case STATUS_BACKUP_IN_PROGRESS:
                result = -2;
                break;

            default:
                result = -1;
                break;
        }
    }

    return (result);
}

const char *Device_Vendor_Name(
    void)
{
    return Vendor_Name;
}

/** Returns the Vendor ID for this Device.
 * See the assignments at http://www.bacnet.org/VendorID/BACnet%20Vendor%20IDs.htm
 * @return The Vendor ID of this Device.
 */
uint16_t Device_Vendor_Identifier(
    void)
{
    return Vendor_Identifier;
}

void Device_Set_Vendor_Identifier(
    uint16_t vendor_id)
{
    Vendor_Identifier = vendor_id;
}

const char *Device_Model_Name(
    void)
{
    return Model_Name;
}

bool Device_Set_Model_Name(
    const char *name,
    size_t length)
{
    bool status = false;        /*return value */

    if (length < sizeof(Model_Name)) {
        memmove(Model_Name, name, length);
        Model_Name[length] = 0;
        status = true;
    }

    return status;
}

const char *Device_Firmware_Revision(
    void)
{
    return BACnet_Version;
}

const char *Device_Application_Software_Version(
    void)
{
    return Application_Software_Version;
}

bool Device_Set_Application_Software_Version(
    const char *name,
    size_t length)
{
    bool status = false;        /*return value */

    if (length < sizeof(Application_Software_Version)) {
        memmove(Application_Software_Version, name, length);
        Application_Software_Version[length] = 0;
        status = true;
    }

    return status;
}

const char *Device_Description(
    void)
{
    return Description;
}

bool Device_Set_Description(
    const char *name,
    size_t length)
{
    bool status = false;        /*return value */

    if (length < sizeof(Description)) {
        memmove(Description, name, length);
        Description[length] = 0;
        status = true;
    }

    return status;
}

const char *Device_Location(
    void)
{
    return Location;
}

bool Device_Set_Location(
    const char *name,
    size_t length)
{
    bool status = false;        /*return value */

    if (length < sizeof(Location)) {
        memmove(Location, name, length);
        Location[length] = 0;
        status = true;
    }

    return status;
}

uint8_t Device_Protocol_Version(
    void)
{
    return BACNET_PROTOCOL_VERSION;
}

uint8_t Device_Protocol_Revision(
    void)
{
    return BACNET_PROTOCOL_REVISION;
}

BACNET_SEGMENTATION Device_Segmentation_Supported(
    void)
{
    return SEGMENTATION_NONE;
}

uint32_t Device_Database_Revision(
    void)
{
    return Database_Revision;
}

void Device_Set_Database_Revision(
    uint32_t revision)
{
    Database_Revision = revision;
}

/*
 * Shortcut for incrementing database revision as this is potentially
 * the most common operation if changing object names and ids is
 * implemented.
 */
void Device_Inc_Database_Revision(
    void)
{
    Database_Revision++;
}

/** Get the total count of objects supported by this Device Object.
 * @note Since many network clients depend on the object list
 *       for discovery, it must be consistent!
 * @return The count of objects, for all supported Object types.
 */
unsigned Device_Object_List_Count(
    void)
{
    unsigned count = 0; /* number of objects */
    struct object_functions *pObject = NULL;

    /* initialize the default return values */
    pObject = Object_Table;
    while (pObject->Object_Type < MAX_BACNET_OBJECT_TYPE) {
        if (pObject->Object_Count) {
            count += pObject->Object_Count();
        }
        pObject++;
    }

    return count;
}

/** Lookup the Object at the given array index in the Device's Object List.
 * Even though we don't keep a single linear array of objects in the Device,
 * this method acts as though we do and works through a virtual, concatenated
 * array of all of our object type arrays.
 *
 * @param array_index [in] The desired array index (1 to N)
 * @param object_type [out] The object's type, if found.
 * @param instance [out] The object's instance number, if found.
 * @return True if found, else false.
 */
bool Device_Object_List_Identifier(
    unsigned array_index,
    int *object_type,
    uint32_t * instance)
{
    bool status = false;
    unsigned count = 0;
    unsigned object_index = 0;
    unsigned temp_index = 0;
    struct object_functions *pObject = NULL;

    /* array index zero is length - so invalid */
    if (array_index == 0) {
        return status;
    }
    object_index = array_index - 1;
    /* initialize the default return values */
    pObject = Object_Table;
    while (pObject->Object_Type < MAX_BACNET_OBJECT_TYPE) {
        if (pObject->Object_Count) {
            object_index -= count;
            count = pObject->Object_Count();
            if (object_index < count) {
                /* Use the iterator function if available otherwise
                 * look for the index to instance to get the ID */
                if (pObject->Object_Iterator) {
                    /* First find the first object */
                    temp_index = pObject->Object_Iterator(~(unsigned) 0);
                    /* Then step through the objects to find the nth */
                    while (object_index != 0) {
                        temp_index = pObject->Object_Iterator(temp_index);
                        object_index--;
                    }
                    /* set the object_index up before falling through to next bit */
                    object_index = temp_index;
                }
                if (pObject->Object_Index_To_Instance) {
                    *object_type = pObject->Object_Type;
                    *instance =
                        pObject->Object_Index_To_Instance(object_index);
                    status = true;
                    break;
                }
            }
        }
        pObject++;
    }

    return status;
}

/** Determine if we have an object with the given object_name.
 * If the object_type and object_instance pointers are not null,
 * and the lookup succeeds, they will be given the resulting values.
 * @param object_name [in] The desired Object Name to look for.
 * @param object_type [out] The BACNET_OBJECT_TYPE of the matching Object.
 * @param object_instance [out] The object instance number of the matching Object.
 * @return True on success or else False if not found.
 */
bool Device_Valid_Object_Name(
    BACNET_CHARACTER_STRING * object_name1,
    int *object_type,
    uint32_t * object_instance)
{
    bool found = false;
    int type = 0;
    uint32_t instance;
    unsigned max_objects = 0, i = 0;
    bool check_id = false;
    BACNET_CHARACTER_STRING object_name2;
    struct object_functions *pObject = NULL;

    max_objects = Device_Object_List_Count();
    for (i = 1; i <= max_objects; i++) {
        check_id = Device_Object_List_Identifier(i, &type, &instance);
        if (check_id) {
            pObject = Device_Objects_Find_Functions(type);
            if ((pObject != NULL) && (pObject->Object_Name != NULL) &&
                (pObject->Object_Name(instance, &object_name2) &&
                    characterstring_same(object_name1, &object_name2))) {
                found = true;
                if (object_type) {
                    *object_type = type;
                }
                if (object_instance) {
                    *object_instance = instance;
                }
                break;
            }
        }
    }

    return found;
}

/** Determine if we have an object of this type and instance number.
 * @param object_type [in] The desired BACNET_OBJECT_TYPE
 * @param object_instance [in] The object instance number to be looked up.
 * @return True if found, else False if no such Object in this device.
 */
bool Device_Valid_Object_Id(
    int object_type,
    uint32_t object_instance)
{
    bool status = false;        /* return value */
    struct object_functions *pObject = NULL;

    pObject = Device_Objects_Find_Functions(object_type);
    if ((pObject != NULL) && (pObject->Object_Valid_Instance != NULL)) {
        status = pObject->Object_Valid_Instance(object_instance);
    }

    return status;
}

/** Copy a child object's object_name value, given its ID.
 * @param object_type [in] The BACNET_OBJECT_TYPE of the child Object.
 * @param object_instance [in] The object instance number of the child Object.
 * @param object_name [out] The Object Name found for this child Object.
 * @return True on success or else False if not found.
 */
bool Device_Object_Name_Copy(
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    struct object_functions *pObject = NULL;
    bool found = false;

    pObject = Device_Objects_Find_Functions(object_type);
    if ((pObject != NULL) && (pObject->Object_Name != NULL)) {
        found = pObject->Object_Name(object_instance, object_name);
    }

    return found;
}

static void Update_Current_Time(
    void)
{
    struct tm *tblock = NULL;
#if defined(_MSC_VER)
    time_t tTemp;
#else
    struct timeval tv;
#endif
/*
struct tm

int    tm_sec   Seconds [0,60].
int    tm_min   Minutes [0,59].
int    tm_hour  Hour [0,23].
int    tm_mday  Day of month [1,31].
int    tm_mon   Month of year [0,11].
int    tm_year  Years since 1900.
int    tm_wday  Day of week [0,6] (Sunday =0).
int    tm_yday  Day of year [0,365].
int    tm_isdst Daylight Savings flag.
*/
#if defined(_MSC_VER)
    time(&tTemp);
    tblock = (struct tm *)localtime(&tTemp);
#else
    if (gettimeofday(&tv, NULL) == 0) {
        tblock = (struct tm *)localtime((const time_t *)&tv.tv_sec);
    }
#endif

    if (tblock) {
        datetime_set_date(&Local_Date, (uint16_t) tblock->tm_year + 1900,
            (uint8_t) tblock->tm_mon + 1, (uint8_t) tblock->tm_mday);
#if !defined(_MSC_VER)
        datetime_set_time(&Local_Time, (uint8_t) tblock->tm_hour,
            (uint8_t) tblock->tm_min, (uint8_t) tblock->tm_sec,
            (uint8_t) (tv.tv_usec / 10000));
#else
        datetime_set_time(&Local_Time, (uint8_t) tblock->tm_hour,
            (uint8_t) tblock->tm_min, (uint8_t) tblock->tm_sec, 0);
#endif
        if (tblock->tm_isdst) {
            Daylight_Savings_Status = true;
        } else {
            Daylight_Savings_Status = false;
        }
        /* note: timezone is declared in <time.h> stdlib. */
        UTC_Offset = timezone / 60;
    } else {
        datetime_date_wildcard_set(&Local_Date);
        datetime_time_wildcard_set(&Local_Time);
        Daylight_Savings_Status = false;
    }
}

void Device_getCurrentDateTime(
    BACNET_DATE_TIME * DateTime)
{
    Update_Current_Time();

    DateTime->date = Local_Date;
    DateTime->time = Local_Time;
}

int32_t Device_UTC_Offset(void)
{
    Update_Current_Time();

    return UTC_Offset;
}

bool Device_Daylight_Savings_Status(void)
{
    return Daylight_Savings_Status;
}

/* return the length of the apdu encoded or BACNET_STATUS_ERROR for error or
   BACNET_STATUS_ABORT for abort message */
int Device_Read_Property_Local(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    int len = 0;        /* apdu len intermediate value */
    BACNET_BIT_STRING bit_string = { 0 };
    BACNET_CHARACTER_STRING char_string = { 0 };
    unsigned i = 0;
    int object_type = 0;
    uint32_t instance = 0;
    unsigned count = 0;
    uint8_t *apdu = NULL;
    struct object_functions *pObject = NULL;
    bool found = false;
    uint16_t apdu_max = 0;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }
    apdu = rpdata->application_data;
    apdu_max = rpdata->application_data_len;
    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len =
                encode_application_object_id(&apdu[0], OBJECT_DEVICE,
                Object_Instance_Number);
            break;
        case PROP_OBJECT_NAME:
            apdu_len =
                encode_application_character_string(&apdu[0], &My_Object_Name);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len = encode_application_enumerated(&apdu[0], OBJECT_DEVICE);
            break;
        case PROP_DESCRIPTION:
            characterstring_init_ansi(&char_string, Description);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_SYSTEM_STATUS:
            apdu_len = encode_application_enumerated(&apdu[0], System_Status);
            break;
        case PROP_VENDOR_NAME:
            characterstring_init_ansi(&char_string, Vendor_Name);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_VENDOR_IDENTIFIER:
            apdu_len =
                encode_application_unsigned(&apdu[0], Vendor_Identifier);
            break;
        case PROP_MODEL_NAME:
            characterstring_init_ansi(&char_string, Model_Name);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_FIRMWARE_REVISION:
            characterstring_init_ansi(&char_string, BACnet_Version);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_APPLICATION_SOFTWARE_VERSION:
            characterstring_init_ansi(&char_string,
                Application_Software_Version);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_LOCATION:
            characterstring_init_ansi(&char_string, Location);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_LOCAL_TIME:
            Update_Current_Time();
            apdu_len = encode_application_time(&apdu[0], &Local_Time);
            break;
        case PROP_UTC_OFFSET:
            Update_Current_Time();
            apdu_len = encode_application_signed(&apdu[0], UTC_Offset);
            break;
        case PROP_LOCAL_DATE:
            Update_Current_Time();
            apdu_len = encode_application_date(&apdu[0], &Local_Date);
            break;
        case PROP_DAYLIGHT_SAVINGS_STATUS:
            Update_Current_Time();
            apdu_len =
                encode_application_boolean(&apdu[0], Daylight_Savings_Status);
            break;
        case PROP_PROTOCOL_VERSION:
            apdu_len =
                encode_application_unsigned(&apdu[0],
                Device_Protocol_Version());
            break;
        case PROP_PROTOCOL_REVISION:
            apdu_len =
                encode_application_unsigned(&apdu[0],
                Device_Protocol_Revision());
            break;
        case PROP_PROTOCOL_SERVICES_SUPPORTED:
            /* Note: list of services that are executed, not initiated. */
            bitstring_init(&bit_string);
            for (i = 0; i < MAX_BACNET_SERVICES_SUPPORTED; i++) {
                /* automatic lookup based on handlers set */
                bitstring_set_bit(&bit_string, (uint8_t) i,
                    apdu_service_supported((BACNET_SERVICES_SUPPORTED) i));
            }
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_PROTOCOL_OBJECT_TYPES_SUPPORTED:
            /* Note: this is the list of objects that can be in this device,
               not a list of objects that this device can access */
            bitstring_init(&bit_string);
            for (i = 0; i < MAX_ASHRAE_OBJECT_TYPE; i++) {
                /* initialize all the object types to not-supported */
                bitstring_set_bit(&bit_string, (uint8_t) i, false);
            }
            /* set the object types with objects to supported */

            pObject = Object_Table;
            while (pObject->Object_Type < MAX_BACNET_OBJECT_TYPE) {
                if ((pObject->Object_Count) && (pObject->Object_Count() > 0)) {
                    bitstring_set_bit(&bit_string, pObject->Object_Type, true);
                }
                pObject++;
            }
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_OBJECT_LIST:
            count = Device_Object_List_Count();
            /* Array element zero is the number of objects in the list */
            if (rpdata->array_index == 0)
                apdu_len = encode_application_unsigned(&apdu[0], count);
            /* if no index was specified, then try to encode the entire list */
            /* into one packet.  Note that more than likely you will have */
            /* to return an error if the number of encoded objects exceeds */
            /* your maximum APDU size. */
            else if (rpdata->array_index == BACNET_ARRAY_ALL) {
                for (i = 1; i <= count; i++) {
                    found =
                        Device_Object_List_Identifier(i, &object_type,
                        &instance);
                    if (found) {
                        len =
                            encode_application_object_id(&apdu[apdu_len],
                            object_type, instance);
                        apdu_len += len;
                        /* assume next one is the same size as this one */
                        /* can we all fit into the APDU? Don't check for last entry */
                        if ((i != count) && (apdu_len + len) >= apdu_max) {
                            /* Abort response */
                            rpdata->error_code =
                                ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
                            apdu_len = BACNET_STATUS_ABORT;
                            break;
                        }
                    } else {
                        /* error: internal error? */
                        rpdata->error_class = ERROR_CLASS_SERVICES;
                        rpdata->error_code = ERROR_CODE_OTHER;
                        apdu_len = BACNET_STATUS_ERROR;
                        break;
                    }
                }
            } else {
                found =
                    Device_Object_List_Identifier(rpdata->array_index,
                    &object_type, &instance);
                if (found) {
                    apdu_len =
                        encode_application_object_id(&apdu[0], object_type,
                        instance);
                } else {
                    rpdata->error_class = ERROR_CLASS_PROPERTY;
                    rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                    apdu_len = BACNET_STATUS_ERROR;
                }
            }
            break;
        case PROP_MAX_APDU_LENGTH_ACCEPTED:
            apdu_len = encode_application_unsigned(&apdu[0], MAX_APDU);
            break;
        case PROP_SEGMENTATION_SUPPORTED:
            apdu_len =
                encode_application_enumerated(&apdu[0],
                Device_Segmentation_Supported());
            break;
        case PROP_APDU_TIMEOUT:
            apdu_len = encode_application_unsigned(&apdu[0], apdu_timeout());
            break;
        case PROP_NUMBER_OF_APDU_RETRIES:
            apdu_len = encode_application_unsigned(&apdu[0], apdu_retries());
            break;
        case PROP_DEVICE_ADDRESS_BINDING:
            apdu_len = address_list_encode(&apdu[0], apdu_max);
            break;
        case PROP_DATABASE_REVISION:
            apdu_len =
                encode_application_unsigned(&apdu[0], Database_Revision);
            break;
#if defined(BACDL_MSTP)
        case PROP_MAX_INFO_FRAMES:
            apdu_len =
                encode_application_unsigned(&apdu[0],
                dlmstp_max_info_frames());
            break;
        case PROP_MAX_MASTER:
            apdu_len =
                encode_application_unsigned(&apdu[0], dlmstp_max_master());
            break;
#endif
        case PROP_ACTIVE_COV_SUBSCRIPTIONS:
            apdu_len = handler_cov_encode_subscriptions(&apdu[0], apdu_max);
            break;
        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }
    /*  only array properties can have array options */
    if ((apdu_len >= 0) && (rpdata->object_property != PROP_OBJECT_LIST) &&
        (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}

/** Looks up the requested Object and Property, and encodes its Value in an APDU.
 * @ingroup ObjIntf
 * If the Object or Property can't be found, sets the error class and code.
 *
 * @param rpdata [in,out] Structure with the desired Object and Property info
 *                 on entry, and APDU message on return.
 * @return The length of the APDU on success, else BACNET_STATUS_ERROR
 */
int Device_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = BACNET_STATUS_ERROR;
    struct object_functions *pObject = NULL;

    /* initialize the default return values */
    rpdata->error_class = ERROR_CLASS_OBJECT;
    rpdata->error_code = ERROR_CODE_UNKNOWN_OBJECT;
    pObject = Device_Objects_Find_Functions(rpdata->object_type);
    if (pObject != NULL) {
        if (pObject->Object_Valid_Instance &&
            pObject->Object_Valid_Instance(rpdata->object_instance)) {
            if (pObject->Object_Read_Property) {
                apdu_len = pObject->Object_Read_Property(rpdata);
            }
        }
    }

    return apdu_len;
}

/* returns true if successful */
bool Device_Write_Property_Local(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value */
    int len = 0;
    BACNET_APPLICATION_DATA_VALUE value;
    int object_type = 0;
    uint32_t object_instance = 0;
    int temp;

    /* decode the some of the request */
    len =
        bacapp_decode_application_data(wp_data->application_data,
        wp_data->application_data_len, &value);
    if (len < 0) {
        /* error while decoding - a value larger than we can handle */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }
    if ((wp_data->object_property != PROP_OBJECT_LIST) &&
        (wp_data->array_index != BACNET_ARRAY_ALL)) {
        /*  only array properties can have array options */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }
    /* FIXME: len < application_data_len: more data? */
    switch (wp_data->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_OBJECT_ID,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                if ((value.type.Object_Id.type == OBJECT_DEVICE) &&
                    (Device_Set_Object_Instance_Number(value.type.
                            Object_Id.instance))) {
                    /* FIXME: we could send an I-Am broadcast to let the world know */
                } else {
                    status = false;
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            }
            break;
        case PROP_NUMBER_OF_APDU_RETRIES:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                /* FIXME: bounds check? */
                apdu_retries_set((uint8_t) value.type.Unsigned_Int);
            }
            break;
        case PROP_APDU_TIMEOUT:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                /* FIXME: bounds check? */
                apdu_timeout_set((uint16_t) value.type.Unsigned_Int);
            }
            break;
        case PROP_VENDOR_IDENTIFIER:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                /* FIXME: bounds check? */
                Device_Set_Vendor_Identifier((uint16_t) value.
                    type.Unsigned_Int);
            }
            break;
        case PROP_SYSTEM_STATUS:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_ENUMERATED,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                temp = Device_Set_System_Status((BACNET_DEVICE_STATUS)
                    value.type.Enumerated, false);
                if (temp != 0) {
                    status = false;
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    if (temp == -1) {
                        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                    } else {
                        wp_data->error_code =
                            ERROR_CODE_OPTIONAL_FUNCTIONALITY_NOT_SUPPORTED;
                    }
                }
            }
            break;
        case PROP_OBJECT_NAME:
            status =
                WPValidateString(&value,
                characterstring_capacity(&My_Object_Name), false,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                /* All the object names in a device must be unique */
                if (Device_Valid_Object_Name(&value.type.Character_String,
                        &object_type, &object_instance)) {
                    if ((object_type == wp_data->object_type) &&
                        (object_instance == wp_data->object_instance)) {
                        /* writing same name to same object */
                        status = true;
                    } else {
                        status = false;
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_DUPLICATE_NAME;
                    }
                } else {
                    Device_Set_Object_Name(&value.type.Character_String);
                }
            }
            break;
        case PROP_LOCATION:
            status =
                WPValidateString(&value, MAX_DEV_LOC_LEN, true,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                Device_Set_Location(characterstring_value(&value.
                        type.Character_String),
                    characterstring_length(&value.type.Character_String));
            }
            break;

        case PROP_DESCRIPTION:
            status =
                WPValidateString(&value, MAX_DEV_DESC_LEN, true,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                Device_Set_Description(characterstring_value(&value.
                        type.Character_String),
                    characterstring_length(&value.type.Character_String));
            }
            break;
        case PROP_MODEL_NAME:
            status =
                WPValidateString(&value, MAX_DEV_MOD_LEN, true,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                Device_Set_Model_Name(characterstring_value(&value.
                        type.Character_String),
                    characterstring_length(&value.type.Character_String));
            }
            break;

        case PROP_MAX_INFO_FRAMES:
#if defined(BACDL_MSTP)
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                if (value.type.Unsigned_Int <= 255) {
                    dlmstp_set_max_info_frames((uint8_t) value.
                        type.Unsigned_Int);
                } else {
                    status = false;
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            }
            break;
#endif
        case PROP_MAX_MASTER:
#if defined(BACDL_MSTP)
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                if ((value.type.Unsigned_Int > 0) &&
                    (value.type.Unsigned_Int <= 127)) {
                    dlmstp_set_max_master((uint8_t) value.type.Unsigned_Int);
                } else {
                    status = false;
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            }
            break;
#endif
        case PROP_OBJECT_TYPE:
        case PROP_VENDOR_NAME:
        case PROP_FIRMWARE_REVISION:
        case PROP_APPLICATION_SOFTWARE_VERSION:
        case PROP_LOCAL_TIME:
        case PROP_UTC_OFFSET:
        case PROP_LOCAL_DATE:
        case PROP_DAYLIGHT_SAVINGS_STATUS:
        case PROP_PROTOCOL_VERSION:
        case PROP_PROTOCOL_REVISION:
        case PROP_PROTOCOL_SERVICES_SUPPORTED:
        case PROP_PROTOCOL_OBJECT_TYPES_SUPPORTED:
        case PROP_OBJECT_LIST:
        case PROP_MAX_APDU_LENGTH_ACCEPTED:
        case PROP_SEGMENTATION_SUPPORTED:
        case PROP_DEVICE_ADDRESS_BINDING:
        case PROP_DATABASE_REVISION:
        case PROP_ACTIVE_COV_SUBSCRIPTIONS:
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

/** Looks up the requested Object and Property, and set the new Value in it,
 *  if allowed.
 * If the Object or Property can't be found, sets the error class and code.
 * @ingroup ObjIntf
 *
 * @param wp_data [in,out] Structure with the desired Object and Property info
 *              and new Value on entry, and APDU message on return.
 * @return True on success, else False if there is an error.
 */
bool Device_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* Ever the pessamist! */
    struct object_functions *pObject = NULL;

    /* initialize the default return values */
    wp_data->error_class = ERROR_CLASS_OBJECT;
    wp_data->error_code = ERROR_CODE_UNKNOWN_OBJECT;
    pObject = Device_Objects_Find_Functions(wp_data->object_type);
    if (pObject != NULL) {
        if (pObject->Object_Valid_Instance &&
            pObject->Object_Valid_Instance(wp_data->object_instance)) {
            if (pObject->Object_Write_Property) {
                status = pObject->Object_Write_Property(wp_data);
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            }
        } else {
            wp_data->error_class = ERROR_CLASS_OBJECT;
            wp_data->error_code = ERROR_CODE_UNKNOWN_OBJECT;
        }
    } else {
        wp_data->error_class = ERROR_CLASS_OBJECT;
        wp_data->error_code = ERROR_CODE_UNKNOWN_OBJECT;
    }

    return (status);
}

/** Looks up the requested Object, and fills the Property Value list.
 * If the Object or Property can't be found, returns false.
 * @ingroup ObjHelpers
 * @param [in] The object type to be looked up.
 * @param [in] The object instance number to be looked up.
 * @param [out] The value list
 * @return True if the object instance supports this feature and value changed.
 */
bool Device_Encode_Value_List(
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance,
    BACNET_PROPERTY_VALUE * value_list)
{
    bool status = false;        /* Ever the pessamist! */
    struct object_functions *pObject = NULL;

    pObject = Device_Objects_Find_Functions(object_type);
    if (pObject != NULL) {
        if (pObject->Object_Valid_Instance &&
            pObject->Object_Valid_Instance(object_instance)) {
            if (pObject->Object_Value_List) {
                status =
                    pObject->Object_Value_List(object_instance, value_list);
            }
        }
    }

    return (status);
}

/** Checks the COV flag in the requested Object
 * @ingroup ObjHelpers
 * @param [in] The object type to be looked up.
 * @param [in] The object instance to be looked up.
 * @return True if the COV flag is set
 */
bool Device_COV(
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance)
{
    bool status = false;        /* Ever the pessamist! */
    struct object_functions *pObject = NULL;

    pObject = Device_Objects_Find_Functions(object_type);
    if (pObject != NULL) {
        if (pObject->Object_Valid_Instance &&
            pObject->Object_Valid_Instance(object_instance)) {
            if (pObject->Object_COV) {
                status = pObject->Object_COV(object_instance);
            }
        }
    }

    return (status);
}

/** Clears the COV flag in the requested Object
 * @ingroup ObjHelpers
 * @param [in] The object type to be looked up.
 * @param [in] The object instance to be looked up.
 */
void Device_COV_Clear(
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance)
{
    struct object_functions *pObject = NULL;

    pObject = Device_Objects_Find_Functions(object_type);
    if (pObject != NULL) {
        if (pObject->Object_Valid_Instance &&
            pObject->Object_Valid_Instance(object_instance)) {
            if (pObject->Object_COV_Clear) {
                pObject->Object_COV_Clear(object_instance);
            }
        }
    }
}

#if defined(INTRINSIC_REPORTING)
void Device_local_reporting(
    void)
{
    struct object_functions *pObject;
    uint32_t objects_count;
    uint32_t object_instance;
    int object_type;
    uint32_t idx;

    objects_count = Device_Object_List_Count();

    /* loop for all objects */
    for (idx = 1; idx < objects_count; idx++) {
        Device_Object_List_Identifier(idx, &object_type, &object_instance);

        pObject = Device_Objects_Find_Functions(object_type);
        if (pObject != NULL) {
            if (pObject->Object_Valid_Instance &&
                pObject->Object_Valid_Instance(object_instance)) {
                if (pObject->Object_Intrinsic_Reporting) {
                    pObject->Object_Intrinsic_Reporting(object_instance);
                }
            }
        }
    }
}
#endif

/** Looks up the requested Object to see if the functionality is supported.
 * @ingroup ObjHelpers
 * @param [in] The object type to be looked up.
 * @return True if the object instance supports this feature.
 */
bool Device_Value_List_Supported(
    BACNET_OBJECT_TYPE object_type)
{
    bool status = false;        /* Ever the pessamist! */
    struct object_functions *pObject = NULL;

    pObject = Device_Objects_Find_Functions(object_type);
    if (pObject != NULL) {
        if (pObject->Object_Value_List) {
            status = true;
        }
    }

    return (status);
}

/** Initialize the Device Object.
 Initialize the group of object helper functions for any supported Object.
 Initialize each of the Device Object child Object instances.
 * @ingroup ObjIntf
 * @param object_table [in,out] array of structure with object functions.
 *  Each Child Object must provide some implementation of each of these
 *  functions in order to properly support the default handlers.
 */
void Device_Init(
    object_functions_t * object_table)
{
    struct object_functions *pObject = NULL;
#if defined(BAC_UCI)
    const char *uciname;
    struct uci_context *ctx;
    fprintf(stderr, "Device_Init\n");
    ctx = ucix_init("bacnet_dev");
    if (!ctx)
        fprintf(stderr, "Failed to load config file bacnet_dev\n");
    uciname = ucix_get_option(ctx, "bacnet_dev", "0", "Name");
    if (uciname != 0) {
        characterstring_init_ansi(&My_Object_Name, uciname);
    } else {
#endif /* defined(BAC_UCI) */
        characterstring_init_ansi(&My_Object_Name, "PiFace Digital Demo");
#if defined(BAC_UCI)
    }
    ucix_cleanup(ctx);
#endif /* defined(BAC_UCI) */

    if (object_table) {
        Object_Table = object_table;
    } else {
        Object_Table = &My_Object_Table[0];
    }
    pObject = Object_Table;
    while (pObject->Object_Type < MAX_BACNET_OBJECT_TYPE) {
        if (pObject->Object_Init) {
            pObject->Object_Init();
        }
        pObject++;
    }
}

bool DeviceGetRRInfo(
    BACNET_READ_RANGE_DATA * pRequest,  /* Info on the request */
    RR_PROP_INFO * pInfo)
{       /* Where to put the response */
    bool status = false;        /* return value */

    switch (pRequest->object_property) {
        case PROP_VT_CLASSES_SUPPORTED:
        case PROP_ACTIVE_VT_SESSIONS:
        case PROP_LIST_OF_SESSION_KEYS:
        case PROP_TIME_SYNCHRONIZATION_RECIPIENTS:
        case PROP_MANUAL_SLAVE_ADDRESS_BINDING:
        case PROP_SLAVE_ADDRESS_BINDING:
        case PROP_RESTART_NOTIFICATION_RECIPIENTS:
        case PROP_UTC_TIME_SYNCHRONIZATION_RECIPIENTS:
            pInfo->RequestTypes = RR_BY_POSITION;
            pRequest->error_class = ERROR_CLASS_PROPERTY;
            pRequest->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            break;

        case PROP_DEVICE_ADDRESS_BINDING:
            pInfo->RequestTypes = RR_BY_POSITION;
            pInfo->Handler = rr_address_list_encode;
            status = true;
            break;

        case PROP_ACTIVE_COV_SUBSCRIPTIONS:
            pInfo->RequestTypes = RR_BY_POSITION;
            pRequest->error_class = ERROR_CLASS_PROPERTY;
            pRequest->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            break;
        default:
            pRequest->error_class = ERROR_CLASS_SERVICES;
            pRequest->error_code = ERROR_CODE_PROPERTY_IS_NOT_A_LIST;
            break;
    }

    return status;
}


#ifdef BAC_ROUTING
/****************************************************************************
 ************* BACnet Routing Functionality (Optional) **********************
 ****************************************************************************
 * The supporting functions are located in gw_device.c, except for those
 * that need access to local data in this file.
 ****************************************************************************/

/** Initialize the first of our array of Devices with the main Device's
 * information, and then swap out some of the Device object functions and
 * replace with ones appropriate for routing.
 * @ingroup ObjIntf
 * @param first_object_instance Set the first (gateway) Device to this
            instance number.
 */
void Routing_Device_Init(
    uint32_t first_object_instance)
{
    struct object_functions *pDevObject = NULL;

    /* Initialize with our preset strings */
    Add_Routed_Device(first_object_instance, &My_Object_Name, Description);

    /* Now substitute our routed versions of the main object functions. */
    pDevObject = Object_Table;
    pDevObject->Object_Index_To_Instance = Routed_Device_Index_To_Instance;
    pDevObject->Object_Valid_Instance =
        Routed_Device_Valid_Object_Instance_Number;
    pDevObject->Object_Name = Routed_Device_Name;
    pDevObject->Object_Read_Property = Routed_Device_Read_Property_Local;
    pDevObject->Object_Write_Property = Routed_Device_Write_Property_Local;
}

#endif /* BAC_ROUTING */


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

bool WPValidateString(
    BACNET_APPLICATION_DATA_VALUE * pValue,
    int iMaxLen,
    bool bEmptyAllowed,
    BACNET_ERROR_CLASS * pErrorClass,
    BACNET_ERROR_CODE * pErrorCode)
{
    pValue = pValue;
    iMaxLen = iMaxLen;
    bEmptyAllowed = bEmptyAllowed;
    pErrorClass = pErrorClass;
    pErrorCode = pErrorCode;

    return false;
}

int handler_cov_encode_subscriptions(
    uint8_t * apdu,
    int max_apdu)
{
    apdu = apdu;
    max_apdu = max_apdu;

    return 0;
}

void testDevice(
    Test * pTest)
{
    bool status = false;
    const char *name = "Patricia";

    status = Device_Set_Object_Instance_Number(0);
    ct_test(pTest, Device_Object_Instance_Number() == 0);
    ct_test(pTest, status == true);
    status = Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE);
    ct_test(pTest, Device_Object_Instance_Number() == BACNET_MAX_INSTANCE);
    ct_test(pTest, status == true);
    status = Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE / 2);
    ct_test(pTest,
        Device_Object_Instance_Number() == (BACNET_MAX_INSTANCE / 2));
    ct_test(pTest, status == true);
    status = Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE + 1);
    ct_test(pTest,
        Device_Object_Instance_Number() != (BACNET_MAX_INSTANCE + 1));
    ct_test(pTest, status == false);


    Device_Set_System_Status(STATUS_NON_OPERATIONAL, true);
    ct_test(pTest, Device_System_Status() == STATUS_NON_OPERATIONAL);

    ct_test(pTest, Device_Vendor_Identifier() == BACNET_VENDOR_ID);

    Device_Set_Model_Name(name, strlen(name));
    ct_test(pTest, strcmp(Device_Model_Name(), name) == 0);

    return;
}

#ifdef TEST_DEVICE
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Device", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testDevice);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_DEVICE */
#endif /* TEST */
