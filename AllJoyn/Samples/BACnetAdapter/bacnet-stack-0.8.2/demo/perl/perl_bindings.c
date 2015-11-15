#include "bacdef.h"
#include "handlers.h"
#include "bacenum.h"
#include "datalink.h"
#include "device.h"
#include <time.h>
#include "arf.h"

/* Free is redefined as a macro, but Perl does not like that. */
#undef free

/* global variables used in this file */
static uint32_t Target_Device_Object_Instance = 4194303;
static unsigned Target_Max_APDU = 0;
static bool Error_Detected = false;
static BACNET_ADDRESS Target_Address;
static uint8_t Request_Invoke_ID = 0;
static bool isReadPropertyHandlerRegistered = false;
static bool isReadPropertyMultipleHandlerRegistered = false;
static bool isWritePropertyHandlerRegistered = false;
static bool isAtomicWriteFileHandlerRegistered = false;
static bool isAtomicReadFileHandlerRegistered = false;

/****************************************/
/* Logging Support */
/****************************************/
#define MAX_ERROR_STRING 128
#define NO_ERROR "No Error"
static char Last_Error[MAX_ERROR_STRING] = NO_ERROR;
static void LogError(
    const char *msg)
{
    strcpy(Last_Error, msg);
    Error_Detected = true;
}

void BacnetGetError(
    SV * errorMsg)
{
    sv_setpv(errorMsg, Last_Error);
    strcpy(Last_Error, NO_ERROR);
    Error_Detected = false;
}

static void __LogAnswer(
    const char *msg,
    unsigned append)
{
    dSP;
    ENTER;
    SAVETMPS;
    PUSHMARK(SP);
    XPUSHs(sv_2mortal(newSVpv(msg, 0)));
    XPUSHs(sv_2mortal(newSViv(append)));
    PUTBACK;
    call_pv("LogAnswer", G_DISCARD);
    FREETMPS;
    LEAVE;
}

/**************************************/
/* error handlers */
/*************************************/
static void MyAbortHandler(
    BACNET_ADDRESS * src,
    uint8_t invoke_id,
    uint8_t abort_reason,
    bool server)
{
    (void) server;
    if (address_match(&Target_Address, src) &&
        (invoke_id == Request_Invoke_ID)) {
        char msg[MAX_ERROR_STRING];
        sprintf(msg, "BACnet Abort: %s",
            bactext_abort_reason_name((int) abort_reason));
        LogError(msg);
    }
}

static void MyRejectHandler(
    BACNET_ADDRESS * src,
    uint8_t invoke_id,
    uint8_t reject_reason)
{
    if (address_match(&Target_Address, src) &&
        (invoke_id == Request_Invoke_ID)) {
        char msg[MAX_ERROR_STRING];
        sprintf(msg, "BACnet Reject: %s",
            bactext_reject_reason_name((int) reject_reason));
        LogError(msg);
    }
}

static void My_Error_Handler(
    BACNET_ADDRESS * src,
    uint8_t invoke_id,
    BACNET_ERROR_CLASS error_class,
    BACNET_ERROR_CODE error_code)
{
    if (address_match(&Target_Address, src) &&
        (invoke_id == Request_Invoke_ID)) {
        char msg[MAX_ERROR_STRING];
        sprintf(msg, "BACnet Error: %s: %s",
            bactext_error_class_name((int) error_class),
            bactext_error_code_name((int) error_code));
        LogError(msg);
    }
}

/**********************************/
/*        ACK handlers            */
/**********************************/

/*****************************************/
/* Decode the ReadProperty Ack and pass to perl */
/****************************************/
#define MAX_ACK_STRING 512
void rp_ack_extract_data(
    BACNET_READ_PROPERTY_DATA * data)
{
    char ackString[MAX_ACK_STRING] = "";
    char *pAckString = &ackString[0];
    BACNET_OBJECT_PROPERTY_VALUE object_value;  /* for bacapp printing */
    BACNET_APPLICATION_DATA_VALUE value;        /* for decode value data */
    int len = 0;
    uint8_t *application_data;
    int application_data_len;
    bool first_value = true;
    bool print_brace = false;

    if (data) {
        application_data = data->application_data;
        application_data_len = data->application_data_len;
        /* FIXME: what if application_data_len is bigger than 255? */
        /* value? need to loop until all of the len is gone... */
        for (;;) {
            len =
                bacapp_decode_application_data(application_data,
                (uint8_t) application_data_len, &value);
            if (first_value && (len < application_data_len)) {
                first_value = false;
                strncat(pAckString, "{", 1);
                pAckString += 1;
                print_brace = true;
            }
            object_value.object_type = data->object_type;
            object_value.object_instance = data->object_instance;
            object_value.object_property = data->object_property;
            object_value.array_index = data->array_index;
            object_value.value = &value;
            bacapp_snprintf_value(pAckString,
                MAX_ACK_STRING - (pAckString - ackString), &object_value);
            if (len > 0) {
                if (len < application_data_len) {
                    application_data += len;
                    application_data_len -= len;
                    /* there's more! */
                    strncat(pAckString, ",", 1);
                    pAckString += 1;
                } else {
                    break;
                }
            } else {
                break;
            }
        }
        if (print_brace) {
            strncat(pAckString, "}", 1);
            pAckString += 1;
        }
        /* Now let's call a Perl function to display the data */
        __LogAnswer(ackString, 0);
    }
}

/*****************************************/
/* Decode the ReadPropertyMultiple Ack and pass to perl */
/****************************************/
void rpm_ack_extract_data(
    BACNET_READ_ACCESS_DATA * rpm_data)
{
    BACNET_OBJECT_PROPERTY_VALUE object_value;  /* for bacapp printing */
    BACNET_PROPERTY_REFERENCE *listOfProperties;
    BACNET_APPLICATION_DATA_VALUE *value;
    bool array_value = false;
    char ackString[MAX_ACK_STRING] = "";
    char *pAckString = &ackString[0];

    if (rpm_data) {
        listOfProperties = rpm_data->listOfProperties;
        while (listOfProperties) {
            value = listOfProperties->value;
            if (value) {
                if (value->next) {
                    strncat(pAckString, "{", 1);
                    pAckString++;
                    array_value = true;
                } else {
                    array_value = false;
                }
                object_value.object_type = rpm_data->object_type;
                object_value.object_instance = rpm_data->object_instance;
                while (value) {
                    object_value.object_property =
                        listOfProperties->propertyIdentifier;
                    object_value.array_index =
                        listOfProperties->propertyArrayIndex;
                    object_value.value = value;
                    bacapp_snprintf_value(pAckString,
                        MAX_ACK_STRING - (pAckString - ackString),
                        &object_value);
                    if (value->next) {
                        strncat(pAckString, ",", 1);
                        pAckString++;
                    } else {
                        if (array_value) {
                            strncat(pAckString, "}", 1);
                            pAckString++;
                        }
                    }
                    value = value->next;
                }
            } else {
                /* AccessError */
                sprintf(ackString, "BACnet Error: %s: %s",
                    bactext_error_class_name((int) listOfProperties->
                        error.error_class),
                    bactext_error_code_name((int) listOfProperties->
                        error.error_code));
                LogError(ackString);
            }
            listOfProperties = listOfProperties->next;

            /* Add a separator between consecutive entries so that Perl can */
            /* parse this out */
            strncat(pAckString, "QQQ", 3);
            pAckString += 3;
        }

        /* Now let's call a Perl function to display the data */
        __LogAnswer(ackString, 1);
    }
}

static void AtomicReadFileAckHandler(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src,
    BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data)
{
    int len = 0;
    BACNET_ATOMIC_READ_FILE_DATA data;

    if (address_match(&Target_Address, src) &&
        (service_data->invoke_id == Request_Invoke_ID)) {
        len =
            arf_ack_decode_service_request(service_request, service_len,
            &data);
        if (len > 0) {
            /* validate the parameters before storing data */
            if ((data.access == FILE_STREAM_ACCESS) &&
                (service_data->invoke_id == Request_Invoke_ID)) {
                char msg[32];
                uint8_t *pFileData;
                int i;

                sprintf(msg, "EOF=%d,start=%d,", data.endOfFile,
                    data.type.stream.fileStartPosition);
                __LogAnswer(msg, 0);

                pFileData = octetstring_value(&data.fileData);
                for (i = 0; i < octetstring_length(&data.fileData); i++) {
                    sprintf(msg, "%02x ", *pFileData);
                    __LogAnswer(msg, 1);
                    pFileData++;
                }
            } else {
                LogError("Bad stream access reported");
            }
        }
    }
}


/** Handler for a ReadProperty ACK.
 * @ingroup DSRP
 * Doesn't actually do anything, except, for debugging, to
 * print out the ACK data of a matching request.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_ADDRESS of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */
static void My_Read_Property_Ack_Handler(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src,
    BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data)
{
    int len = 0;
    BACNET_READ_PROPERTY_DATA data;

    if (address_match(&Target_Address, src) &&
        (service_data->invoke_id == Request_Invoke_ID)) {
        len =
            rp_ack_decode_service_request(service_request, service_len, &data);
        if (len > 0) {
            rp_ack_extract_data(&data);
        }
    }
}

/** Handler for a ReadPropertyMultiple ACK.
 * @ingroup DSRPM
 * For each read property, print out the ACK'd data,
 * and free the request data items from linked property list.
 *
 * @param service_request [in] The contents of the service request.
 * @param service_len [in] The length of the service_request.
 * @param src [in] BACNET_ADDRESS of the source of the message
 * @param service_data [in] The BACNET_CONFIRMED_SERVICE_DATA information
 *                          decoded from the APDU header of this message.
 */
static void My_Read_Property_Multiple_Ack_Handler(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src,
    BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data)
{
    int len = 0;
    BACNET_READ_ACCESS_DATA *rpm_data;
    BACNET_READ_ACCESS_DATA *old_rpm_data;
    BACNET_PROPERTY_REFERENCE *rpm_property;
    BACNET_PROPERTY_REFERENCE *old_rpm_property;
    BACNET_APPLICATION_DATA_VALUE *value;
    BACNET_APPLICATION_DATA_VALUE *old_value;

    if (address_match(&Target_Address, src) &&
        (service_data->invoke_id == Request_Invoke_ID)) {
        rpm_data = calloc(1, sizeof(BACNET_READ_ACCESS_DATA));
        if (rpm_data) {
            len =
                rpm_ack_decode_service_request(service_request, service_len,
                rpm_data);
        }
        if (len > 0) {
            while (rpm_data) {
                rpm_ack_extract_data(rpm_data);
                rpm_property = rpm_data->listOfProperties;
                while (rpm_property) {
                    value = rpm_property->value;
                    while (value) {
                        old_value = value;
                        value = value->next;
                        free(old_value);
                    }
                    old_rpm_property = rpm_property;
                    rpm_property = rpm_property->next;
                    free(old_rpm_property);
                }
                old_rpm_data = rpm_data;
                rpm_data = rpm_data->next;
                free(old_rpm_data);
            }
        } else {
            LogError("RPM Ack Malformed! Freeing memory...");
            while (rpm_data) {
                rpm_property = rpm_data->listOfProperties;
                while (rpm_property) {
                    value = rpm_property->value;
                    while (value) {
                        old_value = value;
                        value = value->next;
                        free(old_value);
                    }
                    old_rpm_property = rpm_property;
                    rpm_property = rpm_property->next;
                    free(old_rpm_property);
                }
                old_rpm_data = rpm_data;
                rpm_data = rpm_data->next;
                free(old_rpm_data);
            }
        }
    }
}

void My_Write_Property_SimpleAck_Handler(
    BACNET_ADDRESS * src,
    uint8_t invoke_id)
{
    if (address_match(&Target_Address, src) &&
        (invoke_id == Request_Invoke_ID)) {
        __LogAnswer("WriteProperty Acknowledged!", 0);
    }
}


static void Init_Service_Handlers(
    )
{
    Device_Init(NULL);

    /* we need to handle who-is to support dynamic device binding to us */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);

    /* handle i-am to support binding to other devices */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, handler_i_am_bind);

    /* set the handler for all the services we don't implement
       It is required to send the proper reject message... */
    apdu_set_unrecognized_service_handler_handler
        (handler_unrecognized_service);

    /* we must implement read property - it's required! */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
        handler_read_property);

    /* handle generic errors coming back */
    apdu_set_abort_handler(MyAbortHandler);
    apdu_set_reject_handler(MyRejectHandler);
}

typedef enum {
    waitAnswer,
    waitBind,
} waitAction;

static void Wait_For_Answer_Or_Timeout(
    unsigned timeout_ms,
    waitAction action)
{
    /* Wait for timeout, failure, or success */
    time_t last_seconds = time(NULL);
    time_t timeout_seconds = (apdu_timeout() / 1000) * apdu_retries();
    time_t elapsed_seconds = 0;
    uint16_t pdu_len = 0;
    BACNET_ADDRESS src = { 0 }; /* address where message came from */
    uint8_t Rx_Buf[MAX_MPDU] = { 0 };

    while (true) {
        time_t current_seconds = time(NULL);

        /* If error was detected then bail out */
        if (Error_Detected) {
            LogError("Some other error occurred");
            break;
        }

        if (elapsed_seconds > timeout_seconds) {
            LogError("APDU Timeout");
            break;
        }

        /* Process PDU if one comes in */
        pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout_ms);
        if (pdu_len) {
            npdu_handler(&src, &Rx_Buf[0], pdu_len);
        }

        /* at least one second has passed */
        if (current_seconds != last_seconds) {
            tsm_timer_milliseconds(((current_seconds - last_seconds) * 1000));
        }

        if (action == waitAnswer) {
            /* Response was received. Exit. */
            if (tsm_invoke_id_free(Request_Invoke_ID)) {
                break;
            } else if (tsm_invoke_id_failed(Request_Invoke_ID)) {
                LogError("TSM Timeout!");
                tsm_free_invoke_id(Request_Invoke_ID);
                break;
            }
        } else if (action == waitBind) {
            if (address_bind_request(Target_Device_Object_Instance,
                    &Target_Max_APDU, &Target_Address)) {
                break;
            }
        } else {
            LogError("Invalid waitAction requested");
            break;
        }

        /* Keep track of time */
        elapsed_seconds += (current_seconds - last_seconds);
        last_seconds = current_seconds;
    }
}

/****************************************************/
/*             Interface API                        */
/****************************************************/

/****************************************************/
/* This is the most fundamental setup needed to start communication  */
/****************************************************/
void BacnetPrepareComm(
    )
{
    /* setup my info */
    Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE);
    address_init();
    Init_Service_Handlers();
    dlenv_init();
}

/****************************************************/
/* Try to bind to a device. If successful, return zero. If failure, return */
/* non-zero and log the error details */
/****************************************************/
int BacnetBindToDevice(
    int deviceInstanceNumber)
{
    int isFailure = 0;

    /* Store the requested device instance number in the global variable for */
    /* reference in other communication routines */
    Target_Device_Object_Instance = deviceInstanceNumber;

    /* try to bind with the device */
    if (!address_bind_request(deviceInstanceNumber, &Target_Max_APDU,
            &Target_Address)) {
        Send_WhoIs(Target_Device_Object_Instance,
            Target_Device_Object_Instance);

        /* Wait for timeout, failure, or success */
        Wait_For_Answer_Or_Timeout(100, waitBind);
    }
    /* Clean up after ourselves */
    isFailure = Error_Detected;
    Error_Detected = false;
    return isFailure;
}

/****************************************************/
/* This is the interface to ReadProperty */
/****************************************************/
int BacnetReadProperty(
    int deviceInstanceNumber,
    int objectType,
    int objectInstanceNumber,
    int objectProperty,
    int objectIndex)
{
    if (!isReadPropertyHandlerRegistered) {
        /* handle the data coming back from confirmed requests */
        apdu_set_confirmed_ack_handler(SERVICE_CONFIRMED_READ_PROPERTY,
            My_Read_Property_Ack_Handler);

        /* handle any errors coming back */
        apdu_set_error_handler(SERVICE_CONFIRMED_READ_PROPERTY,
            My_Error_Handler);

        /* indicate that handlers are now registered */
        isReadPropertyHandlerRegistered = true;
    }
    /* Send the message out */
    Request_Invoke_ID =
        Send_Read_Property_Request(deviceInstanceNumber, objectType,
        objectInstanceNumber, objectProperty, objectIndex);
    Wait_For_Answer_Or_Timeout(100, waitAnswer);

    int isFailure = Error_Detected;
    Error_Detected = 0;
    return isFailure;
}

/************************************************/
/* This is the interface to ReadPropertyMultiple */
/************************************************/
int BacnetReadPropertyMultiple(
    int deviceInstanceNumber,
    ...)
{
    /* Get the variable argument list from the stack */
    Inline_Stack_Vars;
    int rpmIndex = 1;
    BACNET_READ_ACCESS_DATA *rpm_object =
        calloc(1, sizeof(BACNET_READ_ACCESS_DATA));
    BACNET_READ_ACCESS_DATA *Read_Access_Data = rpm_object;
    BACNET_PROPERTY_REFERENCE *rpm_property;
    uint8_t buffer[MAX_PDU] = { 0 };

    while (rpmIndex < Inline_Stack_Items) {
        SV *pSV = Inline_Stack_Item(rpmIndex++);

        /* Make sure the argument is an Array Reference */
        if (SvTYPE(SvRV(pSV)) != SVt_PVAV) {
            LogError("Argument is not an Array reference");
            break;
        }
        /* Make sure we can access the memory */
        if (rpm_object) {
            rpm_object->listOfProperties = NULL;
        } else {
            LogError("Memory Allocation Issue");
            break;
        }

        AV *pAV = (AV *) SvRV(pSV);
        SV **ppSV;

        /* The 0th argument is the object type */
        ppSV = av_fetch(pAV, 0, 0);
        if (ppSV) {
            rpm_object->object_type = SvIV(*ppSV);
        } else {
            LogError("Problem parsing the Array of arguments");
            break;
        }

        /* The 1st argument is the object instance */
        ppSV = av_fetch(pAV, 1, 0);
        if (ppSV) {
            rpm_object->object_instance = SvIV(*ppSV);
        } else {
            LogError("Problem parsing the Array of arguments");
            break;
        }

        /* The 2nd argument is the property type */
        ppSV = av_fetch(pAV, 2, 0);
        if (ppSV) {
            rpm_property = calloc(1, sizeof(BACNET_PROPERTY_REFERENCE));
            rpm_object->listOfProperties = rpm_property;
            if (rpm_property) {
                rpm_property->propertyIdentifier = SvIV(*ppSV);
            } else {
                LogError("Memory allocation error");
                break;
            }
        } else {
            LogError("Problem parsing the Array of arguments");
            break;
        }

        /* The 3rd argument is the property index */
        ppSV = av_fetch(pAV, 3, 0);
        if (ppSV) {
            rpm_property->propertyArrayIndex = SvIV(*ppSV);
        } else {
            LogError("Problem parsing the Array of arguments");
            break;
        }

        /* Advance to the next RPM index */
        if (rpmIndex < Inline_Stack_Items) {
            rpm_object->next = calloc(1, sizeof(BACNET_READ_ACCESS_DATA));
            rpm_object = rpm_object->next;
        } else {
            rpm_object->next = NULL;
        }
    }

    if (!isReadPropertyMultipleHandlerRegistered) {
        /* handle the data coming back from confirmed requests */
        apdu_set_confirmed_ack_handler(SERVICE_CONFIRMED_READ_PROP_MULTIPLE,
            My_Read_Property_Multiple_Ack_Handler);

        /* handle any errors coming back */
        apdu_set_error_handler(SERVICE_CONFIRMED_READ_PROP_MULTIPLE,
            My_Error_Handler);

        /* indicate that handlers are now registered */
        isReadPropertyMultipleHandlerRegistered = true;
    }
    /* Send the message out */
    if (!Error_Detected) {
        Request_Invoke_ID =
            Send_Read_Property_Multiple_Request(&buffer[0], sizeof(buffer),
            deviceInstanceNumber, Read_Access_Data);
        Wait_For_Answer_Or_Timeout(100, waitAnswer);
    }
    /* Clean up allocated memory */
    BACNET_READ_ACCESS_DATA *old_rpm_object;
    BACNET_PROPERTY_REFERENCE *old_rpm_property;

    rpm_object = Read_Access_Data;
    old_rpm_object = rpm_object;
    while (rpm_object) {
        rpm_property = rpm_object->listOfProperties;
        while (rpm_property) {
            old_rpm_property = rpm_property;
            rpm_property = rpm_property->next;
            free(old_rpm_property);
        }
        old_rpm_object = rpm_object;
        rpm_object = rpm_object->next;
        free(old_rpm_object);
    }

    /* Process the return value */
    int isFailure = Error_Detected;
    Error_Detected = 0;
    return isFailure;
}

/****************************************************/
/* This is the interface to WriteProperty */
/****************************************************/
int BacnetWriteProperty(
    int deviceInstanceNumber,
    int objectType,
    int objectInstanceNumber,
    int objectProperty,
    int objectPriority,
    int objectIndex,
    const char *tag,
    const char *value)
{
    char msg[MAX_ERROR_STRING];
    int isFailure = 1;

    if (!isWritePropertyHandlerRegistered) {
        /* handle the ack coming back */
        apdu_set_confirmed_simple_ack_handler(SERVICE_CONFIRMED_WRITE_PROPERTY,
            My_Write_Property_SimpleAck_Handler);

        /* handle any errors coming back */
        apdu_set_error_handler(SERVICE_CONFIRMED_WRITE_PROPERTY,
            My_Error_Handler);

        /* indicate that handlers are now registered */
        isWritePropertyHandlerRegistered = true;
    }

    if (objectIndex == -1) {
        objectIndex = BACNET_ARRAY_ALL;
    }
    /* Loop for eary exit; */
    do {
        /* Handle the tag/value pair */
        uint8_t context_tag = 0;
        BACNET_APPLICATION_TAG property_tag;
        BACNET_APPLICATION_DATA_VALUE propertyValue;

        if (toupper(tag[0]) == 'C') {
            context_tag = strtol(&tag[1], NULL, 0);
            propertyValue.context_tag = context_tag;
            propertyValue.context_specific = true;
        } else {
            propertyValue.context_specific = false;
        }
        property_tag = strtol(tag, NULL, 0);

        if (property_tag >= MAX_BACNET_APPLICATION_TAG) {
            sprintf(msg, "Error: tag=%u - it must be less than %u",
                property_tag, MAX_BACNET_APPLICATION_TAG);
            LogError(msg);
            break;
        }
        if (!bacapp_parse_application_data(property_tag, value,
                &propertyValue)) {
            sprintf(msg, "Error: unable to parse the tag value");
            LogError(msg);
            break;
        }
        propertyValue.next = NULL;

        /* Send out the message */
        Request_Invoke_ID =
            Send_Write_Property_Request(deviceInstanceNumber, objectType,
            objectInstanceNumber, objectProperty, &propertyValue,
            objectPriority, objectIndex);
        Wait_For_Answer_Or_Timeout(100, waitAnswer);

        /* If we get here, then there were no explicit failures. However, there */
        /* could have been implicit failures. Let's look at those also. */
        isFailure = Error_Detected;
    } while (false);

    /* Clean up after ourselves. */
    Error_Detected = false;
    return isFailure;
}


int BacnetAtomicWriteFile(
    int deviceInstanceNumber,
    int fileInstanceNumber,
    int blockStartAddr,
    int blockNumBytes,
    char *nibbleBuffer)
{
    BACNET_OCTET_STRING fileData;
    int i, nibble;
    uint8_t byteValue;
    unsigned char nibbleValue;

    if (!isAtomicWriteFileHandlerRegistered) {
        /* handle any errors coming back */
        apdu_set_error_handler(SERVICE_CONFIRMED_ATOMIC_WRITE_FILE,
            My_Error_Handler);

        /* indicate that handlers are now registered */
        isAtomicWriteFileHandlerRegistered = true;
    }

    for (i = 0; i < blockNumBytes; i++) {
        byteValue = 0;
        for (nibble = 0; nibble < 2; nibble++) {
            nibbleValue = toupper(nibbleBuffer[i * 2 + nibble]);
            if ((nibbleValue >= '0') && (nibbleValue <= '9')) {
                byteValue += (nibbleValue - '0') << (4 * (1 - nibble));
            } else if ((nibbleValue >= 'A') && (nibbleValue <= 'F')) {
                byteValue += (nibbleValue - 'A' + 10) << (4 * (1 - nibble));
            } else {
                LogError("Bad data in buffer.");
            }
        }
        fileData.value[i] = byteValue;
    }
    octetstring_truncate(&fileData, blockNumBytes);

    /* Send out the message and wait for answer */
    if (!Error_Detected) {
        Request_Invoke_ID =
            Send_Atomic_Write_File_Stream(deviceInstanceNumber,
            fileInstanceNumber, blockStartAddr, &fileData);
        Wait_For_Answer_Or_Timeout(100, waitAnswer);
    }

    int isFailure = Error_Detected;
    Error_Detected = 0;
    return isFailure;
}

int BacnetGetMaxApdu(
    )
{
    unsigned requestedOctetCount = 0;
    uint16_t my_max_apdu = 0;

    /* calculate the smaller of our APDU size or theirs
       and remove the overhead of the APDU (varies depending on size).
       note: we could fail if there is a bottle neck (router)
       and smaller MPDU in betweeen. */
    if (Target_Max_APDU < MAX_APDU) {
        my_max_apdu = Target_Max_APDU;
    } else {
        my_max_apdu = MAX_APDU;
    }
    /* Typical sizes are 50, 128, 206, 480, 1024, and 1476 octets */
    if (my_max_apdu <= 50) {
        requestedOctetCount = my_max_apdu - 19;
    } else if (my_max_apdu <= 480) {
        requestedOctetCount = my_max_apdu - 32;
    } else if (my_max_apdu <= 1476) {
        requestedOctetCount = my_max_apdu - 64;
    } else {
        requestedOctetCount = my_max_apdu / 2;
    }

    return requestedOctetCount;
}

int BacnetTimeSync(
    int deviceInstanceNumber,
    int year,
    int month,
    int day,
    int hour,
    int minute,
    int second,
    int isUTC,
    int UTCOffset)
{
    BACNET_DATE bdate;
    BACNET_TIME btime;
    struct tm my_time;
    time_t aTime;
    struct tm *newTime;

    my_time.tm_sec = second;
    my_time.tm_min = minute;
    my_time.tm_hour = hour;
    my_time.tm_mday = day;
    my_time.tm_mon = month - 1;
    my_time.tm_year = year - 1900;
    my_time.tm_wday = 0;        /* does not matter */
    my_time.tm_yday = 0;        /* does not matter */
    my_time.tm_isdst = 0;       /* does not matter */

    aTime = mktime(&my_time);
    newTime = localtime(&aTime);

    bdate.year = newTime->tm_year;
    bdate.month = newTime->tm_mon + 1;
    bdate.day = newTime->tm_mday;
    bdate.wday = newTime->tm_wday ? newTime->tm_wday : 7;
    btime.hour = newTime->tm_hour;
    btime.min = newTime->tm_min;
    btime.sec = newTime->tm_sec;
    btime.hundredths = 0;

    int len = 0;
    int pdu_len = 0;
    int bytes_sent = 0;
    BACNET_NPDU_DATA npdu_data;
    BACNET_ADDRESS my_address;
    uint8_t Handler_Transmit_Buffer[MAX_PDU] = { 0 };

    /* Loop for eary exit */
    do {
        if (!dcc_communication_enabled()) {
            LogError("DCC communicaiton is not enabled");
            break;
        }

        /* encode the NPDU portion of the packet */
        npdu_encode_npdu_data(&npdu_data, false, MESSAGE_PRIORITY_NORMAL);
        datalink_get_my_address(&my_address);
        pdu_len =
            npdu_encode_pdu(&Handler_Transmit_Buffer[0], &Target_Address,
            &my_address, &npdu_data);

        /* encode the APDU portion of the packet */
        len =
            timesync_encode_apdu(&Handler_Transmit_Buffer[pdu_len], &bdate,
            &btime);
        pdu_len += len;

        /* send it out the datalink */
        bytes_sent =
            datalink_send_pdu(&Target_Address, &npdu_data,
            &Handler_Transmit_Buffer[0], pdu_len);
        if (bytes_sent <= 0) {
            char errorMsg[64];
            sprintf(errorMsg,
                "Failed to Send Time-Synchronization Request (%s)!",
                strerror(errno));
            LogError(errorMsg);
            break;
        }

        Wait_For_Answer_Or_Timeout(100, waitAnswer);
    } while (false);

    int isFailure = Error_Detected;
    Error_Detected = 0;
    return isFailure;
}

/****************************************************/
/* This is the interface to AtomicReadFile */
/****************************************************/
int BacnetAtomicReadFile(
    int deviceInstanceNumber,
    int fileInstanceNumber,
    int startOffset,
    int numBytes)
{
    if (!isAtomicReadFileHandlerRegistered) {
        /* handle the data coming back from confirmed requests */
        apdu_set_confirmed_ack_handler(SERVICE_CONFIRMED_ATOMIC_READ_FILE,
            AtomicReadFileAckHandler);

        /* handle any errors coming back */
        apdu_set_error_handler(SERVICE_CONFIRMED_ATOMIC_READ_FILE,
            My_Error_Handler);

        /* indicate that handlers are now registered */
        isAtomicReadFileHandlerRegistered = true;
    }
    /* Send the message out */
    Request_Invoke_ID =
        Send_Atomic_Read_File_Stream(deviceInstanceNumber, fileInstanceNumber,
        startOffset, numBytes);
    Wait_For_Answer_Or_Timeout(100, waitAnswer);

    int isFailure = Error_Detected;
    Error_Detected = 0;
    return isFailure;
}
