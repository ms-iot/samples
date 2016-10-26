/**************************************************************************
*
* Copyright (C) 2008 John Minack
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
*********************************************************************/
#ifndef BACNET_EVENT_H_
#define BACNET_EVENT_H_

#include "bacenum.h"
#include <stdint.h>
#include <stdbool.h>
#include "bacapp.h"
#include "timestamp.h"
#include "bacpropstates.h"
#include "bacdevobjpropref.h"

typedef enum {
    CHANGE_OF_VALUE_BITS,
    CHANGE_OF_VALUE_REAL
} CHANGE_OF_VALUE_TYPE;

/*
** Based on UnconfirmedEventNotification-Request
*/

typedef struct BACnet_Event_Notification_Data {
    uint32_t processIdentifier;
    BACNET_OBJECT_ID initiatingObjectIdentifier;
    BACNET_OBJECT_ID eventObjectIdentifier;
    BACNET_TIMESTAMP timeStamp;
    uint32_t notificationClass;
    uint8_t priority;
    BACNET_EVENT_TYPE eventType;
    BACNET_CHARACTER_STRING *messageText;       /* OPTIONAL - Set to NULL if not being used */
    BACNET_NOTIFY_TYPE notifyType;
    bool ackRequired;
    BACNET_EVENT_STATE fromState;
    BACNET_EVENT_STATE toState;
    /*
     ** Each of these structures in the union maps to a particular eventtype
     ** Based on BACnetNotificationParameters
     */

    union {
        /*
         ** EVENT_CHANGE_OF_BITSTRING
         */
        struct {
            BACNET_BIT_STRING referencedBitString;
            BACNET_BIT_STRING statusFlags;
        } changeOfBitstring;
        /*
         ** EVENT_CHANGE_OF_STATE
         */
        struct {
            BACNET_PROPERTY_STATE newState;
            BACNET_BIT_STRING statusFlags;
        } changeOfState;
        /*
         ** EVENT_CHANGE_OF_VALUE
         */
        struct {
            union {
                BACNET_BIT_STRING changedBits;
                float changeValue;
            } newValue;
            CHANGE_OF_VALUE_TYPE tag;
            BACNET_BIT_STRING statusFlags;
        } changeOfValue;
        /*
         ** EVENT_COMMAND_FAILURE
         **
         ** Not Supported!
         */
        /*
         ** EVENT_FLOATING_LIMIT
         */
        struct {
            float referenceValue;
            BACNET_BIT_STRING statusFlags;
            float setPointValue;
            float errorLimit;
        } floatingLimit;
        /*
         ** EVENT_OUT_OF_RANGE
         */
        struct {
            float exceedingValue;
            BACNET_BIT_STRING statusFlags;
            float deadband;
            float exceededLimit;
        } outOfRange;
        /*
         ** EVENT_CHANGE_OF_LIFE_SAFETY
         */
        struct {
            BACNET_LIFE_SAFETY_STATE newState;
            BACNET_LIFE_SAFETY_MODE newMode;
            BACNET_BIT_STRING statusFlags;
            BACNET_LIFE_SAFETY_OPERATION operationExpected;
        } changeOfLifeSafety;
        /*
         ** EVENT_EXTENDED
         **
         ** Not Supported!
         */
        /*
         ** EVENT_BUFFER_READY
         */
        struct {
            BACNET_DEVICE_OBJECT_PROPERTY_REFERENCE bufferProperty;
            uint32_t previousNotification;
            uint32_t currentNotification;
        } bufferReady;
        /*
         ** EVENT_UNSIGNED_RANGE
         */
        struct {
            uint32_t exceedingValue;
            BACNET_BIT_STRING statusFlags;
            uint32_t exceededLimit;
        } unsignedRange;
    } notificationParams;
} BACNET_EVENT_NOTIFICATION_DATA;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***************************************************
**
** Creates a Confirmed Event Notification APDU
**
****************************************************/
    int cevent_notify_encode_apdu(
        uint8_t * apdu,
        uint8_t invoke_id,
        BACNET_EVENT_NOTIFICATION_DATA * data);

/***************************************************
**
** Creates an Unconfirmed Event Notification APDU
**
****************************************************/
    int uevent_notify_encode_apdu(
        uint8_t * apdu,
        BACNET_EVENT_NOTIFICATION_DATA * data);

/***************************************************
**
** Encodes the service data part of Event Notification
**
****************************************************/
    int event_notify_encode_service_request(
        uint8_t * apdu,
        BACNET_EVENT_NOTIFICATION_DATA * data);

/***************************************************
**
** Decodes the service data part of Event Notification
**
****************************************************/
    int event_notify_decode_service_request(
        uint8_t * apdu,
        unsigned apdu_len,
        BACNET_EVENT_NOTIFICATION_DATA * data);

/***************************************************
**
** Sends an Unconfirmed Event Notifcation to a dest
**
****************************************************/
    int uevent_notify_send(
        uint8_t * buffer,
        BACNET_EVENT_NOTIFICATION_DATA * data,
        BACNET_ADDRESS * dest);


#ifdef __cplusplus
}
#endif /* __cplusplus */
/** @defgroup ALMEVNT Alarm and Event Management BIBBs
 * These BIBBs prescribe the BACnet capabilities required to interoperably
 * perform the alarm and event management functions enumerated in 22.2.1.2
 * for the BACnet devices defined therein.
          *//** @defgroup EVNOTFCN Alarm and Event-Notification (AE-N)
 * @ingroup ALMEVNT
 * 13.6 ConfirmedCOVNotification Service <br>
 * The ConfirmedCOVNotification service is used to notify subscribers about
 * changes that may have occurred to the properties of a particular object.
 * Subscriptions for COV notifications are made using the SubscribeCOV service
 * or the SubscribeCOVProperty service.
 *
 * 13.7 UnconfirmedCOVNotification Service <br>
 * The UnconfirmedCOVNotification Service is used to notify subscribers about
 * changes that may have occurred to the properties of a particular object,
 * or to distribute object properties of wide interest (such as outside air
 * conditions) to many devices simultaneously without a subscription.
 * Subscriptions for COV notifications are made using the SubscribeCOV service.
 * For unsubscribed notifications, the algorithm for determining when to issue
 * this service is a local matter and may be based on a change of value,
 * periodic updating, or some other criteria.
          *//** @defgroup ALMACK  Alarm and Event-ACK (AE-ACK)
 * @ingroup ALMEVNT
 * 13.5 AcknowledgeAlarm Service <br>
 * In some systems a device may need to know that an operator has seen the alarm
 * notification. The AcknowledgeAlarm service is used by a notification-client
 * to acknowledge that a human operator has seen and responded to an event
 * notification with 'AckRequired' = TRUE. Ensuring that the acknowledgment
 * actually comes from a person with appropriate authority is a local matter.
 * This service may be used in conjunction with either the
 * ConfirmedEventNotification service or the UnconfirmedEventNotification service.
 */
#endif /* BACNET_EVENT_H_ */
