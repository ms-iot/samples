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

#include "pch.h"
#include <mutex>
#include "BACnetDef.h"
#include "BACnetObjects.h"
#include "BACnetInterface.h"

using namespace Platform;
using namespace BridgeRT;


//
// COV properties count
//
#ifndef MAX_COV_PROPERTIES
    #define MAX_COV_PROPERTIES 2
#endif

namespace AdapterLib
{
    //
    // IO request codes
    //
    enum IO_TYPE
    {
        IoType_Invalid,
        IoType_StartDeviceDiscovery,
        IoType_ReadDeviceInfo,
        IoType_ReadProperty,
        IoType_WriteProperty,
        IoType_SubscribeCOV,
        IoType_UnsubscribeCOV,

        IoType_MAX
    };


    //
    // BACNET_DEVICE_ID_INTERNAL:
    //  For keeping additional stack specific information.
    //
    struct BACNET_DEVICE_ID_INTERNAL : public BACNET_DEVICE_ID
    {
    public:
        BACNET_DEVICE_ID_INTERNAL()
        {
            RtlZeroMemory(&this->SourceAddress, sizeof(this->SourceAddress));
        }

        // The device sources address
        BACNET_ADDRESS SourceAddress;
    };


    //
    // BACnetServiceHandlers class.
    // Description:
    //      BACnetServiceHandlers is used for hosting
    //      the BACnet service handlers, with reference to the
    //      BACnetInterface object.
    //      We are using a separate callback functions due to:
    //      1. The BACnet stack implementation does not
    //          have a context passed to the handler.
    //      2. We do not want to expose the BACnet stack details
    //          outside the BACnetInterface implementation, to avoid
    //          extra dependency on the specific stack.
    //
    class BACnetServiceHandlers
    {
    public:
        static BACnetServiceHandlers* Instance();
        static void Dispose();

        void Register(BACnetInterface^ BacnetInterface);
        void UnRegister();

        //
        // Service handlers
        //
        static void Error_Handler(
            _In_ BACNET_ADDRESS* SrcAddressPtr,
            _In_ UINT8 InvokeId,
            _In_ BACNET_ERROR_CLASS ErrorClass,
            _In_ BACNET_ERROR_CODE ErrorCode
            );

        static void Abort_Handler(
            _In_ BACNET_ADDRESS* SrcAddressPtr,
            _In_ UINT8 InvokeId,
            _In_ UINT8 AbortReason,
            _In_ bool IsServer
            );

        static void Reject_Handler(
            _In_ BACNET_ADDRESS* SrcAddressPtr,
            _In_ uint8_t InvokeId,
            _In_ uint8_t RejectReason
            );

        static void Iam_Handler(
            _In_count_(ServiceLen) UINT8* ServiceRequestPtr,
            _In_ UINT16 ServiceLen,
            _In_ BACNET_ADDRESS* SrcAddressPtr
            );

        static void ReadPropertyAck_Handler(
            _In_count_(ServiceLen) UINT8* ServiceRequestPtr,
            _In_ UINT16 ServiceLen,
            _In_ BACNET_ADDRESS* SrcAddressPtr,
            _In_ BACNET_CONFIRMED_SERVICE_ACK_DATA* ServiceDataPtr
            );

        static void WritePropertyAck_Handler(
            _In_ BACNET_ADDRESS* SrcAddressPtr,
            _In_ uint8_t InvokeId
            );

        static void SubscribeCovAckHandler(
            _In_ BACNET_ADDRESS* SrcAddressPtr,
            _In_ UINT8 InvokeId
            );

        static void UnconfirmedCovNotification_Handler(
            _In_count_(ServiceLen) UINT8* ServiceRequestPtr,
            _In_ UINT16 ServiceLen,
            _In_ BACNET_ADDRESS* SrcAddressPtr
            );

    private:
        BACnetServiceHandlers();
        virtual ~BACnetServiceHandlers();

    private:

        static std::recursive_mutex lock;
        static BACnetServiceHandlers* instancePtr;

        BACnetInterface^ stackInterface;
    };
    std::recursive_mutex BACnetServiceHandlers::lock;
    BACnetServiceHandlers* BACnetServiceHandlers::instancePtr = nullptr;


    BACnetServiceHandlers*
    BACnetServiceHandlers::Instance()
    {
        AutoLock sync(BACnetServiceHandlers::lock);

        if (BACnetServiceHandlers::instancePtr == nullptr)
        {
            BACnetServiceHandlers::instancePtr = new (std::nothrow) BACnetServiceHandlers();
        }

        return BACnetServiceHandlers::instancePtr;
    }


    void
    BACnetServiceHandlers::Dispose()
    {
        AutoLock sync(BACnetServiceHandlers::lock);

        if (BACnetServiceHandlers::instancePtr != nullptr)
        {
            delete BACnetServiceHandlers::instancePtr; BACnetServiceHandlers::instancePtr = nullptr;
        }
    }


    BACnetServiceHandlers::BACnetServiceHandlers()
    {
    }


    BACnetServiceHandlers::~BACnetServiceHandlers()
    {
        BACnetServiceHandlers::UnRegister();
    }


    void
    BACnetServiceHandlers::Register(BACnetInterface^ BacnetInterface)
    {
        this->stackInterface = BacnetInterface;

        Device_Init(nullptr);

        //
        // Set the handler for all the services we don't implement
        // it is required to send the proper reject message...
        //
        apdu_set_unrecognized_service_handler_handler(
            handler_unrecognized_service
            );

        // We must implement read property - it's required!
        apdu_set_confirmed_handler(
            SERVICE_CONFIRMED_READ_PROPERTY,
            handler_read_property
            );

        // I AM handler
        apdu_set_unconfirmed_handler(
            SERVICE_UNCONFIRMED_I_AM,
            &BACnetServiceHandlers::Iam_Handler
            );

        // Read Property ACK handler
        apdu_set_confirmed_ack_handler(
            SERVICE_CONFIRMED_READ_PROPERTY,
            &BACnetServiceHandlers::ReadPropertyAck_Handler
            );

        // Write Property ACK handler
        apdu_set_confirmed_simple_ack_handler(
            SERVICE_CONFIRMED_WRITE_PROPERTY,
            &BACnetServiceHandlers::WritePropertyAck_Handler
            );

        // COV subscription ACK handler */
        apdu_set_confirmed_simple_ack_handler(
            SERVICE_CONFIRMED_SUBSCRIBE_COV,
            &BACnetServiceHandlers::SubscribeCovAckHandler
            );
        // COV Property subscription ACK handler */
        apdu_set_confirmed_simple_ack_handler(
            SERVICE_CONFIRMED_SUBSCRIBE_COV_PROPERTY,
            &BACnetServiceHandlers::SubscribeCovAckHandler
            );

        // COV handler
        apdu_set_unconfirmed_handler(
            SERVICE_UNCONFIRMED_COV_NOTIFICATION,
            &BACnetServiceHandlers::UnconfirmedCovNotification_Handler
            );

        apdu_set_error_handler(
            SERVICE_CONFIRMED_SUBSCRIBE_COV,
            &BACnetServiceHandlers::Error_Handler
            );
        apdu_set_error_handler(
            SERVICE_CONFIRMED_SUBSCRIBE_COV_PROPERTY,
            &BACnetServiceHandlers::Error_Handler
            );

        // Error handling
        apdu_set_error_handler(
            SERVICE_CONFIRMED_READ_PROPERTY,
            &BACnetServiceHandlers::Error_Handler
            );
        apdu_set_error_handler(
            SERVICE_CONFIRMED_WRITE_PROPERTY,
            &BACnetServiceHandlers::Error_Handler
            );
        apdu_set_error_handler(
            SERVICE_CONFIRMED_SUBSCRIBE_COV,
            &BACnetServiceHandlers::Error_Handler
            );
        apdu_set_abort_handler(
            &BACnetServiceHandlers::Abort_Handler
            );
        apdu_set_reject_handler(
            &BACnetServiceHandlers::Reject_Handler
            );
    }


    void
    BACnetServiceHandlers::UnRegister()
    {
        apdu_set_unrecognized_service_handler_handler(
            nullptr
            );

        apdu_set_confirmed_handler(
            SERVICE_CONFIRMED_READ_PROPERTY,
            nullptr
            );

        apdu_set_unconfirmed_handler(
            SERVICE_UNCONFIRMED_I_AM,
            nullptr
            );

        apdu_set_confirmed_ack_handler(
            SERVICE_CONFIRMED_READ_PROPERTY,
            nullptr
            );

        apdu_set_confirmed_simple_ack_handler(
            SERVICE_CONFIRMED_WRITE_PROPERTY,
            nullptr
            );

        apdu_set_confirmed_simple_ack_handler(
            SERVICE_CONFIRMED_SUBSCRIBE_COV,
            nullptr
            );
        apdu_set_confirmed_simple_ack_handler(
            SERVICE_CONFIRMED_SUBSCRIBE_COV_PROPERTY,
            nullptr
            );

        apdu_set_unconfirmed_handler(
            SERVICE_UNCONFIRMED_COV_NOTIFICATION,
            nullptr
            );

        apdu_set_error_handler(
            SERVICE_CONFIRMED_SUBSCRIBE_COV,
            nullptr
            );
        apdu_set_error_handler(
            SERVICE_CONFIRMED_SUBSCRIBE_COV_PROPERTY,
            nullptr
            );

        apdu_set_error_handler(
            SERVICE_CONFIRMED_READ_PROPERTY,
            nullptr
            );
        apdu_set_abort_handler(
            nullptr
            );
        apdu_set_reject_handler(
            nullptr
            );
    }


    _Use_decl_annotations_
    void
    BACnetServiceHandlers::Error_Handler(
        BACNET_ADDRESS* SrcAddressPtr,
        UINT8 InvokeId,
        BACNET_ERROR_CLASS ErrorClass,
        BACNET_ERROR_CODE ErrorCode
        )
    {
        BACnetServiceHandlers* thisPtr = BACnetServiceHandlers::Instance();

        UNREFERENCED_PARAMETER(SrcAddressPtr);
        UNREFERENCED_PARAMETER(ErrorClass);

        if ((thisPtr == nullptr) || (thisPtr->stackInterface == nullptr))
        {
            DSB_ASSERT(FALSE);
            return;
        }

        BACnetIoRequest^ ioRequest = thisPtr->stackInterface->getStackPendingRequest(UINT32(InvokeId));
        if (ioRequest == nullptr)
        {
            //
            // Request may have been canceled
            //
        }
        else
        {
            ioRequest->Complete(GetWin32Code(ErrorCode), 0);
        }
    }


    _Use_decl_annotations_
    void
    BACnetServiceHandlers::Abort_Handler(
        BACNET_ADDRESS* SrcAddressPtr,
        UINT8 InvokeId,
        UINT8 AbortReason,
        bool IsServer
        )
    {
        BACnetServiceHandlers* thisPtr = BACnetServiceHandlers::Instance();

        UNREFERENCED_PARAMETER(SrcAddressPtr);
        UNREFERENCED_PARAMETER(AbortReason);
        UNREFERENCED_PARAMETER(IsServer);

        if ((thisPtr == nullptr) || (thisPtr->stackInterface == nullptr))
        {
            DSB_ASSERT(FALSE);
            return;
        }

        BACnetIoRequest^ ioRequest = thisPtr->stackInterface->getStackPendingRequest(UINT32(InvokeId));
        if (ioRequest == nullptr)
        {
            //
            // Request may have been canceled
            //
        }
        else
        {
            ioRequest->Complete(ERROR_REQUEST_ABORTED, 0);
        }
    }


    _Use_decl_annotations_
    void
    BACnetServiceHandlers::Reject_Handler(
        BACNET_ADDRESS* SrcAddressPtr,
        UINT8 InvokeId,
        UINT8 RejectReason
        )
    {
        BACnetServiceHandlers* thisPtr = BACnetServiceHandlers::Instance();

        UNREFERENCED_PARAMETER(SrcAddressPtr);
        UNREFERENCED_PARAMETER(RejectReason);

        if ((thisPtr == nullptr) || (thisPtr->stackInterface == nullptr))
        {
            DSB_ASSERT(FALSE);
            return;
        }

        BACnetIoRequest^ ioRequest = thisPtr->stackInterface->getStackPendingRequest(UINT32(InvokeId));
        if (ioRequest == nullptr)
        {
            //
            // Request may have been canceled
            //
        }
        else
        {
            ioRequest->Complete(ERROR_REQUEST_REFUSED, 0);
        }
    }


    _Use_decl_annotations_
    void
    BACnetServiceHandlers::Iam_Handler(
        UINT8* ServiceRequestPtr,
        UINT16 ServiceLen,
        BACNET_ADDRESS* SrcAddressPtr
        )
    {
        BACnetServiceHandlers* thisPtr = BACnetServiceHandlers::Instance();

        UNREFERENCED_PARAMETER(ServiceLen);

        if ((thisPtr == nullptr) || (thisPtr->stackInterface == nullptr))
        {
            DSB_ASSERT(FALSE);
            return;
        }

        BACNET_DEVICE_ID_INTERNAL* deviceIdDescPtr = nullptr;
        unsigned maxApdu = 0;
        int segmentation = 0;

        deviceIdDescPtr = new (std::nothrow) BACNET_DEVICE_ID_INTERNAL();
        if (deviceIdDescPtr == nullptr)
        {
            return;
        }
        deviceIdDescPtr->SourceAddress = *SrcAddressPtr;

        int length = iam_decode_service_request(
                        ServiceRequestPtr,
                        &deviceIdDescPtr->DeviceId,
                        &maxApdu,
                        &segmentation,
                        &deviceIdDescPtr->VendorId
                        );
        if (length != -1)
        {
            address_add(deviceIdDescPtr->DeviceId, maxApdu, SrcAddressPtr);

            thisPtr->stackInterface->addDevice(deviceIdDescPtr);
        }
        else
        {
            delete deviceIdDescPtr;
        }
    }


    _Use_decl_annotations_
    void
    BACnetServiceHandlers::ReadPropertyAck_Handler(
        UINT8* ServiceRequestPtr,
        UINT16 ServiceLen,
        BACNET_ADDRESS* SrcAddressPtr,
        BACNET_CONFIRMED_SERVICE_ACK_DATA* ServiceDataPtr
        )
    {
        DWORD status = ERROR_GEN_FAILURE;
        BACnetServiceHandlers* thisPtr = BACnetServiceHandlers::Instance();
        BACnetIoRequest^ ioRequest;
        BACnetAdapterIoRequest::IO_PARAMETERS ioReqParams;
        const BACNET_OBJECT_PROPERTY_DESCRIPTOR* objPropDescPtr;
        BACnetAdapterValue^ adapterValue;
        std::vector<BACNET_APPLICATION_DATA_VALUE> bacnetValues;
        int length = 0;

        UNREFERENCED_PARAMETER(SrcAddressPtr);

        if ((thisPtr == nullptr) || (thisPtr->stackInterface == nullptr))
        {
            DSB_ASSERT(FALSE);
            goto done;
        }

        // First get the IO request associated with the 'read property' request
        ioRequest = thisPtr->stackInterface->getStackPendingRequest(UINT32(ServiceDataPtr->invoke_id));
        if (ioRequest == nullptr)
        {
            //
            // Request may have been canceled
            //
            goto done;
        }

        // Get access to the caller parameters
        ioRequest->GetIoParameters(&ioReqParams);

        DSB_ASSERT(ioReqParams.InputBufferSize == sizeof(BACNET_OBJECT_PROPERTY_DESCRIPTOR));

        objPropDescPtr = static_cast<const BACNET_OBJECT_PROPERTY_DESCRIPTOR*>(ioReqParams.InputBufferPtr);

        adapterValue = objPropDescPtr->Params.AssociatedAdapterValue;
        if (adapterValue == nullptr)
        {
            status = ERROR_INVALID_HANDLE;
            goto done;
        }

        // Decode the 'read property' data
        BACNET_READ_PROPERTY_DATA readPropData;
        length = rp_ack_decode_service_request(ServiceRequestPtr, ServiceLen, &readPropData);
        if (length == 0)
        {
            status = ERROR_BAD_FORMAT;
            goto done;
        }

        //
        // Decode the received value(s).
        // If the read property is an array, we get multiple
        // values.
        //

        UINT8* appDataPtr = readPropData.application_data;
        int appDataLength = readPropData.application_data_len;

        do
        {
            BACNET_APPLICATION_DATA_VALUE value;

            length = bacapp_decode_application_data(
                        appDataPtr,
                        appDataLength,
                        &value
                        );
            if (length > 0)
            {
                try
                {
                    bacnetValues.push_back(value);

                    appDataPtr += length;
                    appDataLength -= length;
                }
                catch (std::bad_alloc)
                {
                    status = ERROR_NOT_ENOUGH_MEMORY;
                    length = 0;
                    goto done;
                }
            }
            else
            {
                status = ERROR_BAD_FORMAT;
                goto done;
            }

        } // More BACnet values, probably an array...
        while (appDataLength > 0);

        DSB_ASSERT(bacnetValues.size() == 1);

        status = adapterValue->FromBACnet(bacnetValues[0]);

    done:

        if (ioRequest != nullptr)
        {
            ioRequest->Complete(status, 0);
        }
    }


    _Use_decl_annotations_
    void
    BACnetServiceHandlers::WritePropertyAck_Handler(
        BACNET_ADDRESS* SrcAddressPtr,
        UINT8 InvokeId
        )
    {
        BACnetServiceHandlers* thisPtr = BACnetServiceHandlers::Instance();
        DWORD status = ERROR_GEN_FAILURE;
        BACnetIoRequest^ ioRequest;
        BACnetAdapterIoRequest::IO_PARAMETERS ioReqParams;
        const BACNET_OBJECT_PROPERTY_DESCRIPTOR* objPropDescPtr;
        BACNET_ADAPTER_OBJECT_ID propertyObjectId;

        UNREFERENCED_PARAMETER(SrcAddressPtr);

        if ((thisPtr == nullptr) || (thisPtr->stackInterface == nullptr))
        {
            DSB_ASSERT(FALSE);
            goto done;
        }

        // First get the IO request associated with the 'write property' request
        ioRequest = thisPtr->stackInterface->getStackPendingRequest(UINT32(InvokeId));
        if (ioRequest == nullptr)
        {
            //
            // Request may have been canceled
            //
            goto done;
        }

        // Get access to the caller parameters
        ioRequest->GetIoParameters(&ioReqParams);

        DSB_ASSERT(ioReqParams.InputBufferSize == sizeof(BACNET_OBJECT_PROPERTY_DESCRIPTOR));

        objPropDescPtr = static_cast<const BACNET_OBJECT_PROPERTY_DESCRIPTOR*>(ioReqParams.InputBufferPtr);

        propertyObjectId.Bits.Type = objPropDescPtr->ObjectType;
        propertyObjectId.Bits.Instance = objPropDescPtr->ObjectInstance;

        // If this is not a relinquish write, update the cached value
        if (objPropDescPtr->Params.AssociatedBACnetValue.tag != BACNET_APPLICATION_TAG_NULL)
        {
            thisPtr->stackInterface->updatePropertyByValue(
                                        objPropDescPtr->DeviceId,
                                        propertyObjectId,
                                        objPropDescPtr->PropertyId,
                                        objPropDescPtr->Params.AssociatedBACnetValue
                                        );
        }

        status = ERROR_SUCCESS;

    done:

        if (ioRequest != nullptr)
        {
            ioRequest->Complete(status, 0);
        }
    }


    _Use_decl_annotations_
    void
    BACnetServiceHandlers::SubscribeCovAckHandler(
        BACNET_ADDRESS* SrcAddressPtr,
        UINT8 InvokeId
        )
    {
        BACnetServiceHandlers* thisPtr = BACnetServiceHandlers::Instance();
        DWORD status = ERROR_GEN_FAILURE;
        BACnetIoRequest^ ioRequest;

        UNREFERENCED_PARAMETER(SrcAddressPtr);

        if ((thisPtr == nullptr) || (thisPtr->stackInterface == nullptr))
        {
            DSB_ASSERT(FALSE);
            goto done;
        }

        // First get the IO request associated with the 'read property' request
        ioRequest = thisPtr->stackInterface->getStackPendingRequest(UINT32(InvokeId));
        if (ioRequest == nullptr)
        {
            //
            // Request may have been canceled
            //
            goto done;
        }

        status = ERROR_SUCCESS;

    done:

        if (ioRequest != nullptr)
        {
            ioRequest->Complete(status, 0);
        }
    }


    void
    BACnetServiceHandlers::UnconfirmedCovNotification_Handler(
        UINT8* ServiceRequestPtr,
        UINT16 ServiceLen,
        BACNET_ADDRESS* SrcAddressPtr
        )
    {
        BACnetServiceHandlers* thisPtr = BACnetServiceHandlers::Instance();

        UNREFERENCED_PARAMETER(SrcAddressPtr);

        if ((thisPtr == nullptr) || (thisPtr->stackInterface == nullptr))
        {
            DSB_ASSERT(FALSE);
            return;
        }

        //
        // Create a linked list of property values
        //
        BACNET_PROPERTY_VALUE propertyValues[MAX_COV_PROPERTIES];
        RtlZeroMemory(&propertyValues, sizeof(propertyValues));
        for (int propValInx = 0; propValInx < ARRAYSIZE(propertyValues) - 1; ++propValInx)
        {
            propertyValues[propValInx].next = &propertyValues[propValInx + 1];
        }

        BACNET_COV_DATA covData = { 0 };
        covData.listOfValues = &propertyValues[0];

        int length = cov_notify_decode_service_request(
                        ServiceRequestPtr,
                        ServiceLen,
                        &covData
                        );
        if (length <= 0)
        {
            return;
        }

        //
        // Get the present value as DsbVAlue
        //

        DSB_ASSERT(propertyValues[0].propertyIdentifier == PROP_PRESENT_VALUE);

        //
        // Populate the COV event...
        //

        BACNET_ADAPTER_OBJECT_ID objectId(
            covData.monitoredObjectIdentifier.type,
            covData.monitoredObjectIdentifier.instance
            );

        thisPtr->stackInterface->updatePropertyBySignal(
                                    ULONG(covData.initiatingDeviceIdentifier),
                                    objectId,
                                    propertyValues[0].value,
                                    covData.subscriberProcessIdentifier
                                    );
    }



    //
    // BACnetInterface class.
    // Description:
    //      Abstracts the underlying BACnet Stack Interface.
    //
    BACnetInterface::BACnetInterface()
            : isValid(false)
            , bcastAddrPtr(nullptr)
            , serviceHandlersPtr(nullptr)
            , rxThread(this, &BACnetInterface::rxThreadEntry)
            , txThread(this, &BACnetInterface::txThreadEntry)
            , notifyThread(this, &BACnetInterface::notifyThreadEntry)
    {
    }


    BACnetInterface::~BACnetInterface()
    {
        BACnetInterface::Shutdown();
    }


    bool BACnetInterface::IsValid() const
    {
        return this->isValid;
    }


    _Use_decl_annotations_
    DWORD
    BACnetInterface::Initialize(
        const AdapterConfig& ConfigInfo,
		IBACnetNotificationListener^ NotificationListener
        )
    {
        DWORD status = ERROR_SUCCESS;
        std::string networkInterface = ConvertTo<std::string>(ConfigInfo.NetworkInterface);
        const char* networkInterfaceSz = networkInterface.empty() ? nullptr : networkInterface.c_str();
        std::string bbmdAddress = ConvertTo<std::string>(ConfigInfo.BbmdIpAddress);

        this->notificationListener = NotificationListener;

        //
        // Get the broadcast address
        //
        this->bcastAddrPtr = new (std::nothrow) BACNET_ADDRESS();
        if (this->bcastAddrPtr == nullptr)
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }
        RtlZeroMemory(this->bcastAddrPtr, sizeof(BACNET_ADDRESS));
        datalink_get_broadcast_address(this->bcastAddrPtr);

        this->serviceHandlersPtr = BACnetServiceHandlers::Instance();
        if (this->serviceHandlersPtr == nullptr)
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }

        if (!Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE))
        {
            status = ERROR_BAD_CONFIGURATION;
            goto done;
        }

        // Set our BACnet services handlers
        this->serviceHandlersPtr->Register(this);

        address_init();

        // Initialize the data link layer
        if (!datalink_init((char*)networkInterfaceSz))
        {
            status = ERROR_NET_OPEN_FAILED;
            goto done;
        }

        // Register with BBDM server
        if (bvlc_register_with_bbmd(
                UINT32(bip_getaddrbyname((char*)bbmdAddress.c_str())),
                htons(UINT16(ConfigInfo.BbmdIpPort)),
                UINT16(ConfigInfo.BbmdTimetoliveSeconds)
                ) <= 0)
        {
            status = ERROR_NETWORK_UNREACHABLE;
            goto done;
        }

        // Start the TX thread
        status = this->txThread.Start(DEF_THREAD_START_TIMEOUT_MSEC);
        if (status != ERROR_SUCCESS)
        {
            goto done;
        }

        // Start the RX thread
        status = this->rxThread.Start(DEF_THREAD_START_TIMEOUT_MSEC);
        if (status != ERROR_SUCCESS)
        {
            goto done;
        }

        // Start the notification thread
        status = this->notifyThread.Start(DEF_THREAD_START_TIMEOUT_MSEC);
        if (status != ERROR_SUCCESS)
        {
            goto done;
        }

        // Initialize() successfully completed!
        this->isValid = true;

    done:

        if (status == ERROR_SUCCESS)
        {
            this->configInfo = ConfigInfo;
        }
        else
        {
            this->Shutdown();
        }

        return status;
    }


    DWORD
    BACnetInterface::Shutdown()
    {
        DWORD status = ERROR_SUCCESS;

        // Stop the threads
        this->notifyThread.Stop();
        this->txThread.Stop();
        this->rxThread.Stop();

        if (this->isValid)
        {
            datalink_cleanup();
        }

        delete this->bcastAddrPtr;  this->bcastAddrPtr = nullptr;

        // UnRegister service handlers
        if (this->serviceHandlersPtr != nullptr)
        {
            this->serviceHandlersPtr->Dispose();
        }
        this->serviceHandlersPtr = nullptr;

        // Release all device ID descriptors
        while (this->deviceDb.size() != 0)
        {
            auto iter = this->deviceDb.begin();
            BACNET_DEVICE_ID_INTERNAL* deviceIdPtr = static_cast<BACNET_DEVICE_ID_INTERNAL*>(iter->second);

            this->deviceDb.erase(iter);
            delete deviceIdPtr;
        }

        // Clean all pending requests
        while (this->pendingStackRequests.size() != 0)
        {
            auto iter = this->pendingStackRequests.begin();
            BACnetIoRequest^ ioReq = iter->second;

            this->pendingStackRequests.erase(iter);

            ioReq->Complete(ERROR_CANCELLED, 0);
        }

        this->isValid = false;

        return status;
    }


    _Use_decl_annotations_
    DWORD
    BACnetInterface::EnumDevices(
        BACnetAdapterIoRequest::COMPLETE_REQUEST_HANDLER CompletionRoutinePtr,
        PVOID ContextPtr,
        IAdapterIoRequest^* adapterIoRequestPtr
        )
    {
        DWORD status = 0;
        BACnetIoRequest^ bacnetIoRequest = nullptr;

        {
            AutoLock sync(this->lock);

            if (adapterIoRequestPtr != nullptr)
            {
                *adapterIoRequestPtr = nullptr;
            }

            //
            // Set IO parameters and submit the request
            //

            BACnetAdapterIoRequest::IO_PARAMETERS ioParams;
            ioParams.Type = IoType_StartDeviceDiscovery;

            IAdapterIoRequest^ ioRequest;
            status = this->doIo(&ioParams, CompletionRoutinePtr, ContextPtr, &ioRequest);
            if (status != ERROR_IO_PENDING)
            {
                //
                // Something probably failed, otherwise
                // we should get ERROR_IO_PENDING since
                // we only queue the request for device enumeration.
                //
                DSB_ASSERT(status != ERROR_SUCCESS);

                return status;
            }

            bacnetIoRequest = dynamic_cast<BACnetIoRequest^>(ioRequest);
            DSB_ASSERT(bacnetIoRequest);

            if (adapterIoRequestPtr != nullptr)
            {
                *adapterIoRequestPtr = ioRequest;

                return ERROR_IO_PENDING;
            }
            else
            {
                // Since we implicitly asked for the request
                bacnetIoRequest->Dereference();
            }
        }        

        status = bacnetIoRequest->Wait(INFINITE, NULL);
        if (status == ERROR_SUCCESS)
        {
            status = bacnetIoRequest->Status();
        }
        else
        {
            DSB_ASSERT(status == ERROR_REQUEST_ABORTED);
        }

        return status;
    }


    _Use_decl_annotations_
    DWORD
    BACnetInterface::ReadObjectProperty(
        const BACNET_OBJECT_PROPERTY_DESCRIPTOR* ObjectPropDescPtr,
        BACnetAdapterIoRequest::COMPLETE_REQUEST_HANDLER CompletionRoutinePtr,
        PVOID ContextPtr,
        IAdapterIoRequest^* adapterIoRequestPtr
        )
    {
        //
        // Set IO parameters
        //
        BACnetAdapterIoRequest::IO_PARAMETERS ioParams;
        ioParams.Type = IoType_ReadProperty;
        ioParams.InputBufferPtr = ObjectPropDescPtr;
        ioParams.InputBufferSize = sizeof(BACNET_OBJECT_PROPERTY_DESCRIPTOR);

        // Create and submit the IO request
        return this->doIo(&ioParams, CompletionRoutinePtr, ContextPtr, adapterIoRequestPtr);
    }


    _Use_decl_annotations_
    DWORD
    BACnetInterface::WriteObjectProperty(
        const BACNET_OBJECT_PROPERTY_DESCRIPTOR* ObjectPropDescPtr,
        BACnetAdapterIoRequest::COMPLETE_REQUEST_HANDLER CompletionRoutinePtr,
        PVOID ContextPtr,
        IAdapterIoRequest^* adapterIoRequestPtr
        )
    {
        //
        // Set IO parameters
        //
        BACnetAdapterIoRequest::IO_PARAMETERS ioParams;
        ioParams.Type = IoType_WriteProperty;
        ioParams.InputBufferPtr = ObjectPropDescPtr;
        ioParams.InputBufferSize = sizeof(BACNET_OBJECT_PROPERTY_DESCRIPTOR);

        // Create and submit the IO request
        return this->doIo(&ioParams, CompletionRoutinePtr, ContextPtr, adapterIoRequestPtr);
    }


    _Use_decl_annotations_
    DWORD
    BACnetInterface::SubscribeCOVProperty(
        const BACNET_OBJECT_PROPERTY_DESCRIPTOR* ObjectPropDescPtr,
        BACnetAdapterIoRequest::COMPLETE_REQUEST_HANDLER CompletionRoutinePtr,
        PVOID ContextPtr,
        IAdapterIoRequest^* adapterIoRequestPtr
        )
    {
        BACnetAdapterIoRequest::IO_PARAMETERS ioParams;
        ioParams.Type = IoType_SubscribeCOV;
        ioParams.InputBufferPtr = ObjectPropDescPtr;
        ioParams.InputBufferSize = sizeof(BACNET_OBJECT_PROPERTY_DESCRIPTOR);

        // Create and submit the IO request
        return this->doIo(&ioParams, CompletionRoutinePtr, ContextPtr, adapterIoRequestPtr);
    }


    _Use_decl_annotations_
    DWORD
    BACnetInterface::UnsubscribeCOVProperty(
        const BACNET_OBJECT_PROPERTY_DESCRIPTOR* ObjectPropDescPtr,
        BACnetAdapterIoRequest::COMPLETE_REQUEST_HANDLER CompletionRoutinePtr,
        PVOID ContextPtr,
        IAdapterIoRequest^* adapterIoRequestPtr
        )
    {
        BACnetAdapterIoRequest::IO_PARAMETERS ioParams;
        ioParams.Type = IoType_UnsubscribeCOV;
        ioParams.InputBufferPtr = ObjectPropDescPtr;
        ioParams.InputBufferSize = sizeof(BACNET_OBJECT_PROPERTY_DESCRIPTOR);

        // Create and submit the IO request
        return this->doIo(&ioParams, CompletionRoutinePtr, ContextPtr, adapterIoRequestPtr);
    }


    _Use_decl_annotations_
    bool
    BACnetInterface::addDevice(BACNET_DEVICE_ID* newDeviceIdPtr)
    {
        // Sync access to device database
        AutoLock sync(this->lock);
        bool isNewDevice = false;

        try
        {
            // Add the new device to device database
            auto insertRes = this->deviceDb.insert(
                std::pair<UINT32, BACNET_DEVICE_ID*>(newDeviceIdPtr->DeviceId, newDeviceIdPtr)
                );

            // Get access to the device ID
            BACNET_DEVICE_ID* deviceIdPtr = insertRes.first->second;

            // Is it a new device ?
            if (insertRes.second == true)
            {
                //
                // Notify listener about the new device.
                //

                BACNET_EVENT_PARAMETERS^ eventParams = ref new BACNET_EVENT_PARAMETERS();
                eventParams->Type = BACNET_EVENT_PARAMETERS::EventType_NewDevice;
                eventParams->AsNewDevice.DeviceId = newDeviceIdPtr->DeviceId;
                eventParams->AsNewDevice.VendorId = newDeviceIdPtr->VendorId;

                this->pendingNotifications.Add(eventParams);

                isNewDevice = true;
            }

            // Update the time
            ::GetLocalTime(&deviceIdPtr->Updated);
        }
        catch (std::bad_alloc)
        {
            DSB_ASSERT(FALSE);
        }

        return isNewDevice;
    }


    _Use_decl_annotations_
    void
    BACnetInterface::updatePropertyBySignal(
        UINT32 DeviceId,
        const BACNET_ADAPTER_OBJECT_ID& PropertyObjectId,
        const BACNET_APPLICATION_DATA_VALUE& PresentValue,
        int SignalHandle
        )
    {
        //
        // Notify listener about the new device.
        //

        BACNET_EVENT_PARAMETERS^ eventParams = ref new BACNET_EVENT_PARAMETERS();
        eventParams->Type = BACNET_EVENT_PARAMETERS::EventType_ChangeOfValue;
        eventParams->AsCOV.DeviceId = DeviceId;
        eventParams->AsCOV.PropertyObectId = PropertyObjectId.Ulong;
        eventParams->AsCOV.SignalHandle = SignalHandle;
        eventParams->AsCOV.PresentValue = PresentValue;

        this->pendingNotifications.Add(eventParams);
    }


    _Use_decl_annotations_
    void
    BACnetInterface::updatePropertyByValue(
        UINT32 DeviceId,
        const BACNET_ADAPTER_OBJECT_ID& PropertyObjectId,
        BACNET_PROPERTY_ID  PropertyId,
        const BACNET_APPLICATION_DATA_VALUE& PresentValue
        )
    {
        BACNET_EVENT_PARAMETERS^ eventParams = ref new BACNET_EVENT_PARAMETERS();
        eventParams->Type = BACNET_EVENT_PARAMETERS::EventType_SetValueAck;
        eventParams->AsSetValueAck.DeviceId = DeviceId;
        eventParams->AsSetValueAck.PropertyObectId = PropertyObjectId.Ulong;
        eventParams->AsSetValueAck.BACnetPorpertyId = UINT32(PropertyId);
        eventParams->AsSetValueAck.CurrentValue = PresentValue;

        this->pendingNotifications.Add(eventParams);
    }


    //
    //  Routine Description:
    //      doIo() is the common IO dispatch routine, it performs the following tasks:
    //      1) Allocate and initialize and BACnetIoRequest IO request object:
    //          - Request parameters.
    //          - Optional completion routine
    //      2) Add the request to the TX queue, and either wait for completion,
    //          or return the IO request object to the caller to wait on.
    //          The IO request processing is picked up by the TX thread.
    //
    //  Arguments:
    //
    //      IoParamsPtr - Request parameters
    //      CompletionRoutinePtr - Optional completion routine.
    //      ContextPtr - Optional completion routine context to be passed to
    //          the completion routine.
    //
    //  Return Value:
    //      - ERROR_SUCCESS: Request was successfully completed (Sync IO).
    //      - ERROR_IO_PENDING: Request was successfully queued (Async IO).
    //      - ERROR_NOT_ENOUGH_MEMORY: Failed to allocate IO request.
    //      - Request completion code.
    //
    _Use_decl_annotations_
    DWORD
    BACnetInterface::doIo(
        const BACnetAdapterIoRequest::IO_PARAMETERS* IoParamsPtr,
        BACnetAdapterIoRequest::COMPLETE_REQUEST_HANDLER CompletionRoutinePtr,
        PVOID ContextPtr,
        IAdapterIoRequest^* adapterIoRequestPtr
        )
    {
        DWORD status = ERROR_SUCCESS;
        BACnetIoRequest^ request;

        request = this->adapterIoRequestPool.Alloc<BACnetIoRequest>(this);
        if (request == nullptr)
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }

        request->Initialialize(IoParamsPtr, nullptr);
        request->SetCancelRoutine(&BACnetInterface::onCancelIo);
        request->SetCompletionRoutine(CompletionRoutinePtr, ContextPtr);

        //
        // Either us or the caller need access to the request, boost the
        // reference count, so it does not go away, before we, or the caller can
        // inspect it.
        //
        request->Reference();

        request->MarkPending();

        status = ERROR_IO_PENDING;

        //
        // Queue the request.
        // Processing of the IO request is picked up by the TX thread.
        //
        if (this->txThreadWorkQueue.Add(request) != ERROR_SUCCESS)
        {
            status = ERROR_NOT_ENOUGH_MEMORY;

            this->completeIoRequest(request, status);
            goto done;
        }

        // If caller uses sync IO, wait for completion
        if (adapterIoRequestPtr == nullptr)
        {
            // Sync IO
            status = request->Wait(DEF_IO_REQ_TIMEOUT_MSEC);
            if (status != ERROR_SUCCESS)
            {
                this->completeIoRequest(request, status);
            }
            else
            {
                status = request->GetStatus(nullptr);
            }
        }

    done:

        if (adapterIoRequestPtr != nullptr)
        {
            *adapterIoRequestPtr = request;
        }
        else if (request != nullptr)
        {
            request->Dereference();
        }

        return status;
    }


    BACnetIoRequest^
    BACnetInterface::getStackPendingRequest(UINT32 RequestId)
    {
        AutoLock sync(this->pendingStackRequestsLock);

        auto iter = this->pendingStackRequests.find(RequestId);

        if (iter == this->pendingStackRequests.end())
        {
            return nullptr;
        }

        BACnetIoRequest^ ioReq = iter->second;

        this->pendingStackRequests.erase(iter);

        return ioReq;
    }


    uint32
    BACnetInterface::putStackPendingRequest(UINT32 RequestId, BACnetIoRequest^ Request)
    {
        DWORD status = ERROR_SUCCESS;

        try
        {
            auto insertRes = this->pendingStackRequests.insert(
                std::pair<UINT32, BACnetIoRequest^>(RequestId, Request)
                );

            if (insertRes.second == false)
            {
                //
                // Already in!
                //
                DSB_ASSERT(FALSE);

                status = ERROR_DUPLICATE_TAG;
            }
        }
        catch (std::bad_alloc)
        {
            status = ERROR_NOT_ENOUGH_MEMORY;
        }

        return status;
    }


    uint32
    BACnetInterface::delStackPendingRequest(BACnetIoRequest^ Request)
    {
        AutoLock sync(this->pendingStackRequestsLock);

        for (auto iter = this->pendingStackRequests.begin();
             iter != this->pendingStackRequests.end();
             iter++)
        {
            BACnetIoRequest^ ioReq = iter->second;

            if (ioReq == Request)
            {
                this->pendingStackRequests.erase(iter);
                return ERROR_SUCCESS;
            }
        }

        return ERROR_NOT_FOUND;
    }


    //
    // The network listener thread uses the stack to read/decode incoming
    // BACnet network messages and dispatch execution to registered handler.
    // RX thread runs until BACnetInterface is shut down.
    //
    DWORD
    BACnetInterface::rxThreadEntry()
    {
        this->rxThread.SetStartStatus(ERROR_SUCCESS);

        //
        // Wait and process incoming messages, until we shut down.
        //
        while (::WaitForSingleObjectEx(this->rxThread.GetStopEvent(), 0, FALSE) == WAIT_TIMEOUT)
        {
            // Address of origin device
            BACNET_ADDRESS srcAddress = { 0 };
            uint8_t rxBuffer[MAX_MPDU] = { 0 };

            uint16_t pduLength = datalink_receive(
                                    &srcAddress,
                                    &rxBuffer[0],
                                    sizeof(rxBuffer),
                                    this->configInfo.RxPacketTimeoutMsec
                                    );
            if (pduLength != 0)
            {
                npdu_handler(&srcAddress, &rxBuffer[0], pduLength);
            }
        }

        return ERROR_SUCCESS;
    }


    //
    // The TX thread waits for TX requests and uses the stack
    // to send the associated BACnet message.
    // TX thread runs until BACnetInterface is shut down.
    //
    DWORD
    BACnetInterface::txThreadEntry()
    {
        HANDLE events[2] = { this->txThread.GetStopEvent(), this->txThreadWorkQueue.GetNotEmptyEvent() };

        this->txThread.SetStartStatus(ERROR_SUCCESS);

        bool isDone = false;

        while (!isDone)
        {
            DWORD waitStatus = ::WaitForMultipleObjectsEx(ARRAYSIZE(events), events, FALSE, INFINITE, FALSE);

            switch (waitStatus)
            {
            case WAIT_OBJECT_0 + 0:
                isDone = true;
                break;

            case WAIT_OBJECT_0 + 1:
            {
                this->onTxWorkItem(this->txThreadWorkQueue.GetNext());
                break;
            }

            default:
                DSB_ASSERT(FALSE);
                break;
            }
        }

        // Cleanup pending events
        while (this->txThreadWorkQueue.Count() != 0)
        {
            BACnetIoRequest^ request = this->txThreadWorkQueue.GetNext();

            request->Complete(ERROR_CANCELLED, 0);
        }

        return ERROR_SUCCESS;
    }


    void
    BACnetInterface::onTxWorkItem(BACnetIoRequest^ BACnetAdapterIoRequest)
    {
        switch (BACnetAdapterIoRequest->GetType())
        {
        case IoType_StartDeviceDiscovery:
            this->txEnumDevices(BACnetAdapterIoRequest);
            break;

        case IoType_ReadProperty:
            this->txReadProperty(BACnetAdapterIoRequest);
            break;

        case IoType_WriteProperty:
            this->txWriteProperty(BACnetAdapterIoRequest);
            break;

        case IoType_SubscribeCOV:
            this->txSubscribeProperty(BACnetAdapterIoRequest, true);
            break;

        case IoType_UnsubscribeCOV:
            this->txSubscribeProperty(BACnetAdapterIoRequest, false);
            break;
        }
    }


    void
    BACnetInterface::txEnumDevices(BACnetIoRequest^ BACnetAdapterIoRequest)
    {
        // See who is out there...
        Send_WhoIs_To_Network(
            this->bcastAddrPtr,
            this->configInfo.DeviceInstanceMin,
            this->configInfo.DeviceInstanceMax
            );

        BACnetAdapterIoRequest->Complete(ERROR_SUCCESS, 0);
    }


    void
    BACnetInterface::txReadProperty(BACnetIoRequest^ BACnetAdapterIoRequest)
    {
        DWORD status = ERROR_SUCCESS;
        BACnetAdapterIoRequest::IO_PARAMETERS ioParams;
        BACnetAdapterIoRequest->GetIoParameters(&ioParams);

        DSB_ASSERT(ioParams.InputBufferSize == sizeof(BACNET_OBJECT_PROPERTY_DESCRIPTOR));

        // Get access to the read property parameters
        const BACNET_OBJECT_PROPERTY_DESCRIPTOR* objPropDescPtr =
            reinterpret_cast<const BACNET_OBJECT_PROPERTY_DESCRIPTOR*>(ioParams.InputBufferPtr);

        // Make sure we 'know' this device
        BACNET_ADDRESS deviceAddress;
        unsigned maxApdu;
        bool isDeviceBound = address_bind_request(
                                objPropDescPtr->DeviceId,
                                &maxApdu,
                                &deviceAddress
                                );
        if (isDeviceBound)
        {
            //
            // We need to lock the 'pending requests' list in order
            // to avoid a race condition where the handler is called before
            // the request is added to the 'pending requests' list.
            //
            AutoLock sync(this->pendingStackRequestsLock);

            BACnetAdapterIoRequest->InvokeId = Send_Read_Property_Request(
                                        objPropDescPtr->DeviceId,
                                        objPropDescPtr->ObjectType,
                                        objPropDescPtr->ObjectInstance,
                                        objPropDescPtr->PropertyId,
                                        objPropDescPtr->ValueIndex
                                        );
            if (BACnetAdapterIoRequest->InvokeId == 0)
            {
                status = ERROR_DEVICE_NOT_AVAILABLE;
                goto done;
            }

            status = this->putStackPendingRequest(BACnetAdapterIoRequest->InvokeId, BACnetAdapterIoRequest);
            if (status == ERROR_SUCCESS)
            {
                status = ERROR_IO_PENDING;
            }
        }
        else
        {
            status = ERROR_DEVICE_NOT_AVAILABLE;
        }

    done:

        if (status != ERROR_IO_PENDING)
        {
            BACnetAdapterIoRequest->Complete(status, 0);
        }
    }


    void
    BACnetInterface::txWriteProperty(BACnetIoRequest^ BACnetAdapterIoRequest)
    {
        DWORD status = ERROR_SUCCESS;
        BACnetAdapterIoRequest::IO_PARAMETERS ioParams;
        BACnetAdapterIoRequest->GetIoParameters(&ioParams);

        DSB_ASSERT(ioParams.InputBufferSize == sizeof(BACNET_OBJECT_PROPERTY_DESCRIPTOR));

        // Get access to the read property parameters
        const BACNET_OBJECT_PROPERTY_DESCRIPTOR* objPropDescPtr =
            reinterpret_cast<const BACNET_OBJECT_PROPERTY_DESCRIPTOR*>(ioParams.InputBufferPtr);

        // Make sure we 'know' this device
        BACNET_ADDRESS deviceAddress;
        unsigned maxApdu;
        bool isDeviceBound = address_bind_request(
                                objPropDescPtr->DeviceId,
                                &maxApdu,
                                &deviceAddress
                                );
        if (isDeviceBound)
        {
            //
            // We need to lock the 'pending requests' list in order
            // to avoid a race condition where the handler is called before
            // the request is added to the 'pending requests' list.
            //
            AutoLock sync(this->pendingStackRequestsLock);

            BACnetAdapterIoRequest->InvokeId = Send_Write_Property_Request(
                                        objPropDescPtr->DeviceId,
                                        objPropDescPtr->ObjectType,
                                        objPropDescPtr->ObjectInstance,
                                        objPropDescPtr->PropertyId,
                                        (BACNET_APPLICATION_DATA_VALUE*)&objPropDescPtr->Params.AssociatedBACnetValue,
                                        this->configInfo.RequestPriority,
                                        BACNET_ARRAY_ALL // All array
                                        );
            if (BACnetAdapterIoRequest->InvokeId == 0)
            {
                status = ERROR_DEVICE_NOT_AVAILABLE;
                goto done;
            }

            status = this->putStackPendingRequest(BACnetAdapterIoRequest->InvokeId, BACnetAdapterIoRequest);
            if (status == ERROR_SUCCESS)
            {
                status = ERROR_IO_PENDING;
            }
        }
        else
        {
            status = ERROR_DEVICE_NOT_AVAILABLE;
        }

    done:

        if (status != ERROR_IO_PENDING)
        {
            BACnetAdapterIoRequest->Complete(status, 0);
        }
    }


    void
    BACnetInterface::txSubscribeProperty(BACnetIoRequest^ BACnetAdapterIoRequest, bool IsSubscribe)
    {
        DWORD status = ERROR_SUCCESS;
        BACnetAdapterIoRequest::IO_PARAMETERS ioParams;
        BACnetAdapterIoRequest->GetIoParameters(&ioParams);

        DSB_ASSERT(ioParams.InputBufferSize == sizeof(BACNET_OBJECT_PROPERTY_DESCRIPTOR));

        // Get access to the read property parameters
        const BACNET_OBJECT_PROPERTY_DESCRIPTOR* objPropDescPtr =
            reinterpret_cast<const BACNET_OBJECT_PROPERTY_DESCRIPTOR*>(ioParams.InputBufferPtr);

        // Make sure we 'know' this device
        BACNET_ADDRESS deviceAddress;
        unsigned maxApdu;
        bool isDeviceBound = address_bind_request(
                                objPropDescPtr->DeviceId,
                                &maxApdu,
                                &deviceAddress
                                );
        if (isDeviceBound)
        {
            //
            // We need to lock the 'pending requests' list in order
            // to avoid a race condition where the handler is called before
            // the request is added to the 'pending requests' list.
            //
            AutoLock sync(this->pendingStackRequestsLock);

            BACNET_SUBSCRIBE_COV_DATA covData = { 0 };
            covData.monitoredObjectIdentifier.type = UINT16(objPropDescPtr->ObjectType);
            covData.monitoredObjectIdentifier.instance = objPropDescPtr->ObjectInstance;
            covData.monitoredProperty.propertyIdentifier = objPropDescPtr->PropertyId;
            covData.monitoredProperty.propertyArrayIndex = BACNET_ARRAY_ALL;
            covData.cancellationRequest = !IsSubscribe;
            covData.issueConfirmedNotifications = false;
            covData.subscriberProcessIdentifier = UINT32(objPropDescPtr->Params.AssociatedAdapterSignal);

            BACnetAdapterIoRequest->InvokeId = Send_COV_Subscribe(
                                        objPropDescPtr->DeviceId,
                                        &covData
                                        );
            if (BACnetAdapterIoRequest->InvokeId == 0)
            {
                status = ERROR_DEVICE_NOT_AVAILABLE;
                goto done;
            }

            status = this->putStackPendingRequest(BACnetAdapterIoRequest->InvokeId, BACnetAdapterIoRequest);
            if (status == ERROR_SUCCESS)
            {
                status = ERROR_IO_PENDING;
            }
        }
        else
        {
            status = ERROR_DEVICE_NOT_AVAILABLE;
        }

    done:

        if (status != ERROR_IO_PENDING)
        {
            BACnetAdapterIoRequest->Complete(status, 0);
        }
    }


    //
    // The notify thread waits for queued notifications and
    // passes them to the notification listener.
    // Notification thread runs until BACnetInterface is shut down.
    //
    DWORD
    BACnetInterface::notifyThreadEntry()
    {
        HANDLE events[2] = { this->notifyThread.GetStopEvent(), this->pendingNotifications.GetNotEmptyEvent() };

        this->notifyThread.SetStartStatus(ERROR_SUCCESS);

        bool isDone = false;

        while (!isDone)
        {
            DWORD waitStatus = ::WaitForMultipleObjectsEx(ARRAYSIZE(events), events, FALSE, INFINITE, FALSE);

            switch (waitStatus)
            {
            case WAIT_OBJECT_0 + 0:
                isDone = true;
                break;

            case WAIT_OBJECT_0 + 1:
            {
                //
                // Notify listener, if any...
                //
                BACNET_EVENT_PARAMETERS^ eventParams = this->pendingNotifications.GetNext();

                if (this->notificationListener != nullptr)
                {
                    this->notificationListener->OnEvent(eventParams);
                }

                break;

            } // Notify listener

            default:
                DSB_ASSERT(FALSE);
                break;
            }

        } // waitStatus

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    bool
    BACnetInterface::onCancelIo(
        IAdapterIoRequest^ Request
        )
    {
        BACnetIoRequest^ bacnetIoRequest = dynamic_cast<BACnetIoRequest^>(Request);

        DSB_ASSERT(bacnetIoRequest != nullptr);

        BACnetInterface^ bacnetIfc = dynamic_cast<BACnetInterface^>(bacnetIoRequest->GetParent());
        if (bacnetIfc == nullptr)
        {
            DSB_ASSERT(FALSE);
        }

        bacnetIfc->completeIoRequest(bacnetIoRequest, ERROR_CANCELLED);

        return true;
    }


    void
    BACnetInterface::completeIoRequest(
        BACnetIoRequest^ Request,
        DWORD status
        )
    {
        AutoLock sync(this->lock);

        Request->SetCancelRoutine(nullptr);

        //
        // Make sure we can complete the request here,
        // since it may have already been completed.
        //

        if (this->delStackPendingRequest(Request) == ERROR_SUCCESS)
        {
            Request->Complete(status, 0);
        }
        else if (this->txThreadWorkQueue.Remove(Request, nullptr))
        {
            Request->Complete(status, 0);
        }
        else
        {
            // IO request already completed!
        }
    }

} // namespace AdapterLib
