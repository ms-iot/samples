// Copyright (c) 2015, Microsoft Corporation
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

#pragma once

#include <vector>
#include <map>

#include "Misc.h"
#include "Thread.h"
#include "BACnetAdapterIoRequest.h"
#include "BACnetNotification.h"
#include "BACnetAdapterDevice.h"
#include "AdapterConfig.h"

namespace AdapterLib
{

    //
    // Forward declarations of BACnet stack
    // types
    //
    struct BACnet_Device_Address;

    //
    // BACNET_DEVICE_ID:
    //  BACnet device identification descriptor
    //
    struct BACNET_DEVICE_ID
    {
        //
        // The 32 bit device ID:
        // <object type (bit# 31-22) : instance (bit# 21-0)>
        //
        UINT32      DeviceId;

        //
        // The vendor identifier
        //
        UINT16      VendorId;

        //
        // The time the device was updated
        //
        SYSTEMTIME  Updated;

        BACNET_DEVICE_ID()
            : DeviceId(UINT32(-1))
            , VendorId(UINT16(-1))
        {
            RtlZeroMemory(&this->Updated, sizeof(this->Updated));
        }
    };


    //
    // BACNET_OBJECT_PROPERTY_DESCRIPTOR:
    //  BACnet object property identification descriptor
    //
    struct BACNET_OBJECT_PROPERTY_DESCRIPTOR
    {
        //
        // The hosting BACnet device ID, acquired
        // during device discovery with BACNET_DEVICE_ID.
        //
        UINT32              DeviceId;

        //
        // The desired object type,
        // OBJECT_ANALOG_INPUT, OBJECT_ANALOG_OUTPUT, etc.
        //
        BACNET_OBJECT_TYPE  ObjectType;

        //
        // The desired object instance.
        // OBJECT_ANALOG_INPUT, OBJECT_ANALOG_OUTPUT, etc.
        //
        UINT32              ObjectInstance;

        //
        // The ID of the target property:
        // PROP_PRESENT_VALUE, PROP_OBJECT_NAME, etc.
        //
        BACNET_PROPERTY_ID  PropertyId;

        //
        // When reading an array:
        // - 0 for the array size
        // - 1 to n for individual array members
        // - BACNET_ARRAY_ALL (~0) for the full array to be read.
        //
        UINT32              ValueIndex;

        struct
        {
            //
            // The HANDLE of the associated adapter value
            //
            BACnetAdapterValue^ AssociatedAdapterValue;

            //
            // The BACnet value associated with the property
            //
            BACNET_APPLICATION_DATA_VALUE AssociatedBACnetValue;

            //
            // The HANDLE of the associated adapter signal
            //
            int AssociatedAdapterSignal;

        } Params;
    };


    //
    // BACNET_EVENT_PARAMS:
    //  BACnet event parameters.
    //
    public ref struct BACNET_EVENT_PARAMETERS sealed
    {
    internal:

        // The event type codes
        enum TYPE
        {
            EventType_Invalid,
            EventType_NewDevice,
            EventType_ChangeOfValue,
            EventType_SetValueAck,
        };

        // Event type
        TYPE Type;

        // Event parameters
        union
        {
            // New Device notification
            struct DEVICE_ID
            {
                UINT32      DeviceId;
                UINT16      VendorId;

            } AsNewDevice;

            // Change of value
            struct CHANGE_OF_VALUE
            {
                UINT32  DeviceId;
                ULONG   PropertyObectId;
                int     SignalHandle;
                BACNET_APPLICATION_DATA_VALUE PresentValue;

            } AsCOV;

            // Set value acknowledge
            struct SET_VALUE_ACK
            {
                UINT32  DeviceId;
                ULONG   PropertyObectId;
                UINT32  BACnetPorpertyId;
                BACNET_APPLICATION_DATA_VALUE CurrentValue;

            } AsSetValueAck;
        };
    };


    //
    // BACnetIoRequest class.
    // Description:
    //  An IO request with BACnet specific information
    //
    ref class BACnetInterface;
    ref class BACnetIoRequest : public BACnetAdapterIoRequest
    {
        friend class BACnetAdapterIoRequestPool;
        friend ref class BACnetInterface;

    protected private:
        BACnetIoRequest(_In_ BACnetAdapterIoRequestPool* PoolPtr, _In_ Platform::Object^ Parent)
            : BACnetAdapterIoRequest(PoolPtr, Parent)
            , InvokeId(0)
        {
        }
        ~BACnetIoRequest()
        {
        }

        virtual void reInitialize(Platform::Object^ Parent)
        {
            BACnetAdapterIoRequest::reInitialize(Parent);

            this->InvokeId = 0;
        }

        UINT32 InvokeId;
    };


    //
    // BACnetInterface class.
    // Description:
    //  Abstracts the underlying BACnet Stack Interface.
    //
    class BACnetServiceHandlers;
    ref class BACnetInterface
    {
        friend class BACnetServiceHandlers;
        friend ref class BACnetAdapter;

    internal:
        DWORD Initialize(
            _In_ const AdapterConfig& ConfigInfo,
            _In_ IBACnetNotificationListener^ NotificationListener
            );
        DWORD Shutdown();

        DWORD EnumDevices(
            _In_opt_ BACnetAdapterIoRequest::COMPLETE_REQUEST_HANDLER CompletionRoutinePtr,
            _In_opt_ PVOID ContextPtr,
            _Out_opt_ BridgeRT::IAdapterIoRequest^* adapterIoRequestPtr
            );

        DWORD ReadObjectProperty(
            _In_ const BACNET_OBJECT_PROPERTY_DESCRIPTOR* ObjectPropDescPtr,
            _In_opt_ BACnetAdapterIoRequest::COMPLETE_REQUEST_HANDLER CompletionRoutinePtr,
            _In_opt_ PVOID ContextPtr,
            _Out_opt_ BridgeRT::IAdapterIoRequest^* adapterIoRequestPtr
            );
        DWORD WriteObjectProperty(
            _In_ const BACNET_OBJECT_PROPERTY_DESCRIPTOR* ObjectPropDescPtr,
            _In_opt_ BACnetAdapterIoRequest::COMPLETE_REQUEST_HANDLER CompletionRoutinePtr,
            _In_opt_ PVOID ContextPtr,
            _Out_opt_ BridgeRT::IAdapterIoRequest^* adapterIoRequestPtr
            );

        DWORD SubscribeCOVProperty(
            _In_ const BACNET_OBJECT_PROPERTY_DESCRIPTOR* ObjectPropDescPtr,
            _In_opt_ BACnetAdapterIoRequest::COMPLETE_REQUEST_HANDLER CompletionRoutinePtr,
            _In_opt_ PVOID ContextPtr,
            _Out_opt_ BridgeRT::IAdapterIoRequest^* adapterIoRequestPtr
            );
        DWORD UnsubscribeCOVProperty(
            _In_ const BACNET_OBJECT_PROPERTY_DESCRIPTOR* ObjectPropDescPtr,
            _In_opt_ BACnetAdapterIoRequest::COMPLETE_REQUEST_HANDLER CompletionRoutinePtr,
            _In_opt_ PVOID ContextPtr,
            _Out_opt_ BridgeRT::IAdapterIoRequest^* adapterIoRequestPtr
            );

        bool IsValid() const;

    protected private:
        BACnetInterface();
        ~BACnetInterface();

    private:

        // Sync object
        DsbCommon::CSLock lock;

        // If interface was initialized successfully
        bool isValid;

        //
        // The BACNet network address to send broadcast
        // messages to.
        //
        BACNET_ADDRESS* bcastAddrPtr;

        // Service handlers object
        BACnetServiceHandlers*  serviceHandlersPtr;

        // The configuration information
        AdapterConfig configInfo;

        // The device database
        std::map<UINT32, BACNET_DEVICE_ID*> deviceDb;

        //
        // Network RX/TX threads
        //
        DsbCommon::MemberThread<BACnetInterface> rxThread;
        DsbCommon::MemberThread<BACnetInterface> txThread;

        void txEnumDevices(BACnetIoRequest^ BACnetAdapterIoRequest);
        void txReadProperty(BACnetIoRequest^ BACnetAdapterIoRequest);
        void txWriteProperty(BACnetIoRequest^ BACnetAdapterIoRequest);
        void txSubscribeProperty(BACnetIoRequest^ BACnetAdapterIoRequest, bool IsSubscribe);

        // The TX thread task queue
        DsbCommon::QueueEx<BACnetIoRequest^> txThreadWorkQueue;

        // DSB IO request pool
        BACnetAdapterIoRequestPool adapterIoRequestPool;

        //
        // Notification area
        //

        // The notification thread
        DsbCommon::MemberThread<BACnetInterface> notifyThread;

        // The notification thread queue
        DsbCommon::QueueEx<BACNET_EVENT_PARAMETERS^> pendingNotifications;

        // The notification listener
        IBACnetNotificationListener^ notificationListener;

        // Requests that are pending stack response
        std::map<UINT32, BACnetIoRequest^> pendingStackRequests;

        // A lock object for pandingStackRequests
        DsbCommon::CSLock pendingStackRequestsLock;

    protected private:

        BACnetIoRequest^ getStackPendingRequest(UINT32 RequestId);
        uint32 putStackPendingRequest(UINT32 RequestId, BACnetIoRequest^ RequestPtr);
        uint32 delStackPendingRequest(BACnetIoRequest^ RequestPtr);

        bool addDevice(_In_ BACNET_DEVICE_ID* newDeviceIdPtr);
        void updatePropertyBySignal(
            _In_ UINT32 DeviceId,
            _In_ const AdapterLib::BACNET_ADAPTER_OBJECT_ID& PropertyObjectId,
            _In_ const BACNET_APPLICATION_DATA_VALUE& PresentValue,
            _In_ int SignalHandle
            );
        void updatePropertyByValue(
            _In_ UINT32 DeviceId,
            _In_ const AdapterLib::BACNET_ADAPTER_OBJECT_ID& PropertyObjectId,
            _In_ BACNET_PROPERTY_ID  PropertyId,
            _In_ const BACNET_APPLICATION_DATA_VALUE& PresentValue
            );

        DWORD doIo(
            _In_ const BACnetAdapterIoRequest::IO_PARAMETERS* IoParamsPtr,
            _In_opt_ BACnetAdapterIoRequest::COMPLETE_REQUEST_HANDLER CompletionRoutinePtr,
            _In_opt_ PVOID ContextPtr,
            _Out_opt_ BridgeRT::IAdapterIoRequest^* adapterIoRequestPtr
            );

        void completeIoRequest(
            BACnetIoRequest^ Request,
            DWORD status
            );

    private:

        DWORD   rxThreadEntry();

        DWORD   txThreadEntry();
        void    onTxWorkItem(BACnetIoRequest^ adapterIoRequestPtr);

        DWORD   notifyThreadEntry();

        static  BACnetAdapterIoRequest::CANCEL_REQUEST onCancelIo;
    };

} // namespace AdapterLib