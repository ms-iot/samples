//
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

#include "AdapterDefinitions.h"
#include "IControlPanelHandler.h"
#include "ILSFHandler.h"

namespace BridgeRT
{
    //Access type enum
    public enum class E_ACCESS_TYPE
    {
        ACCESS_READ,
        ACCESS_WRITE,
        ACCESS_READWRITE
    };

    //Type of Change of Value Signal emitted
    public enum class SignalBehavior
    {
        Unspecified,         //COV signal emit unreliable
        Never,               //Never emits COV
        Always,              //Emits COV on value change
        AlwaysWithNoValue    //emits COV without the value
    };


    //
    // IAdapterValue interface.
    // Description:
    //  Interface of an Adapter Value information.
    //
    public interface class IAdapterValue
    {
        // Object name
        property Platform::String^ Name
        {
            Platform::String^ get();
        }

        //
        //  Property Description:
        //      The DSB values comes in the shape of Platform::Object.
        //
        //      - Setting a value:
        //        Caller sets an IAdapterValue.Data with a Platform::Object^
        //        that was created with Windows.Foundation::PropertyValue::Create???().
        //        Example:
        //          Object^ boolObj = PropertyValue::CreateBoolean(true);
        //          // dsbValue is IAdapterValue^
        //          dsbValue->Data = boolObj;
        //
        //      - Getting a value:
        //          IPropertyValue^ ipv = dynamic_cast<IPropertyValue^>(adapterValue->Data);
        //          if (ipv != nullptr)
        //          {
        //              switch (ipv->Type)
        //              {
        //              case PropertyType::Boolean:
        //                  {
        //                      bool boolValue = ipv->GetBoolean();
        //
        //                      return boolValue;
        //                  }
        //              ...
        //              }
        //          }
        //          else
        //          {
        //              // adapterValue->Data is an object, use dynamic_cast<> to get access.
        //          }
        //
        property Platform::Object^ Data
        {
            Platform::Object^ get();

            void set(Platform::Object^ NewData);
        }
    };

    //
    // IAdapterAttribute interface.
    // Description:
    //  Interface of an Adapter Attribute information.
    //
    public interface class IAdapterAttribute
    {
        // Adapter Value Object
        property IAdapterValue^ Value
        {
            IAdapterValue^ get();
        }

        // Annotations for the property
        //
        // Annotations are the list of name-value pairs that represent
        // the metadata or additional information for the attribute
        // Example: min value, max value, range, units, etc.
        //
        // These values do not change for different instances of the same 
        // property interface.
        //
        // If the value remains unchanged for the lifetime of an object, however, 
        // can be different for different objects, then the value cannot be
        // represented as an annotation. In such case, it must be a separate attribute 
        // for the same property interface with the COVBehavior as SignalBehavior::Never
        //
        property IAnnotationMap^ Annotations
        {
            IAnnotationMap^ get();
        }

        // Access
        property E_ACCESS_TYPE Access
        {
            E_ACCESS_TYPE get();
        }

        // Change of Value (COV) signal supported
        //
        // The following values are supported:
        //
        // Unspecified      : The COV signal behavior is not consistent.
        //                    Used when the signal is raised sometimes but not always.
        // Never            : The COV signal is never raised.
        //                    Used for constant/read only attributes.
        // Always           : The COV signal is raised whenever the attribute changes.
        //                    The new value is included in the payload for the signal.
        // AlwaysWithNoValue : The COV signal is raised whenever the attribute changes.
        //                    However, the new value is not included as part of the signal payload.
        //
        property SignalBehavior COVBehavior
        {
            SignalBehavior get();
        }
    };


    //
    // IAdapterProperty interface.
    // Description:
    //  Interface of an Adapter Property information.
    //
    public interface class IAdapterProperty
    {
        // Object name
        property Platform::String^ Name
        {
            Platform::String^ get();
        }

        // hint for the interface name
        property Platform::String^ InterfaceHint
        {
            Platform::String^ get();
        }

        // The bag of attributes
        property IAdapterAttributeVector^ Attributes
        {
            IAdapterAttributeVector^ get();
        }
    };


    //
    // IAdapterMethod interface.
    // Description:
    //  Interface of an Adapter Method information.
    //
    public interface class IAdapterMethod
    {
        // Object name
        property Platform::String^ Name
        {
            Platform::String^ get();
        }

        // Method description
        property Platform::String^ Description
        {
            Platform::String^ get();
        }

        // The input parameters
        property IAdapterValueVector^ InputParams
        {
            IAdapterValueVector^ get();
            void set(IAdapterValueVector^ Params);
        }

        // The output parameters
        property IAdapterValueVector^ OutputParams
        {
            IAdapterValueVector^ get();
        }

        // The return value
        property int32 HResult
        {
            int32 get();
        }
    };


    //
    // IAdapterSignal interface.
    // Description:
    //  Interface of an Adapter Signal information.
    //
    public interface class IAdapterSignal
    {
        // Object name
        property Platform::String^ Name
        {
            Platform::String^ get();
        }

        // The signal parameters
        property IAdapterValueVector^ Params
        {
            IAdapterValueVector^ get();
        }
    };


    //
    // IAdapterSignalListener interface.
    // Description:
    //  Any object who needs to intercept adapter signals implements IAdapterSignalListener
    //  and implements AdapterSignalHandler() method.
    //
    public interface class IAdapterSignalListener
    {
        void AdapterSignalHandler(
            _In_ IAdapterSignal^ Signal,
            _In_opt_ Platform::Object^ Context
            );
    };

    //
    // IAdapterIcon interface.
    // Description:
    //      Supporting interface for the AllJoyn About Icon.  An ICON is associated with AdapterDevice.
    //      An ICON may consist of a small image not exceeding 128K bytes, or and URL or both.
    //      The MIME Type *MUST* be present.  If both an image and and URL are present, then the
    //      MIME type MUST be the same for both.
    //      
    //      
    public interface class IAdapterIcon
    {
        // An Array of Image Data
        virtual Platform::Array<BYTE>^ GetImage() = 0;

        // The Mime Type of the Image data e.g. image/png, etc
        // Return empty-string/null if not used.  
        property Platform::String^ MimeType
        {
            Platform::String^ get();
        }

        // An Optional URL to that points to an About Icon.
        // Return empty-string/null if not used
        property Platform::String^ Url
        {
            Platform::String^ get();
        }

    };



    //
    // IAdapterDevice interface.
    // Description:
    //  Interface of an Adapter Device information.
    //
    public interface class IAdapterDevice
    {
        // Object name
        property Platform::String^ Name
        {
            Platform::String^ get();
        }

        //
        // Device information
        //
        property Platform::String^ Vendor
        {
            Platform::String^ get();
        }
        property Platform::String^ Model
        {
            Platform::String^ get();
        }
        property Platform::String^ Version
        {
            Platform::String^ get();
        }
        property Platform::String^ FirmwareVersion
        {
            Platform::String^ get();
        }
        property Platform::String^ SerialNumber
        {
            Platform::String^ get();
        }
        property Platform::String^ Description
        {
            Platform::String^ get();
        }

        // Device properties
        property IAdapterPropertyVector^ Properties
        {
            IAdapterPropertyVector^ get();
        }

        // Device methods
        property IAdapterMethodVector^ Methods
        {
            IAdapterMethodVector^ get();
        }

        // Device signals
        property IAdapterSignalVector^ Signals
        {
            IAdapterSignalVector^ get();
        }

        property IAdapterIcon^ Icon
        {
            IAdapterIcon^ get();
        }
    };



    //
    // IAdapterDeviceControlPanel interface.
    // Description:
    //  IAdapterDevice extension for supporting 
    //  a Control Panel
    //
    public interface class IAdapterDeviceControlPanel
    {
        property IControlPanelHandler^ ControlPanelHandler
        {
            IControlPanelHandler^ get();
        }
    };



    //
    // IAdapterDeviceLightingService interface.
    // Description:
    //  IAdapterDevice extension for supporting 
    //  the Lighting Service
    //
    public interface class IAdapterDeviceLightingService
    {
        property ILSFHandler^ LightingServiceHandler
        {
            ILSFHandler^ get();
        }
    };



    //
    // IAdapterIoRequest interface.
    // Description:
    //  Interface of an Adapter IoRequest.
    //
    public interface class IAdapterIoRequest
    {
        //
        //  Routine Description:
        //      Status is called by the  Bridge component to get the current IO
        //      request status.
        //
        //  Arguments:
        //
        //  Return Value:
        //      ERROR_SUCCESS,
        //      ERROR_INVALID_HANDLE: Invalid request handle.
        //      ERROR_IO_PENDING: The request is pending.
        //      ERROR_REQUEST_ABORTED: Request was canceled, or wait was aborted.
        //
        uint32  Status();

        //
        //  Routine Description:
        //      Wait is called by the  Bridge component to wait for a pending
        //      request.
        //
        //  Arguments:
        //
        //      TimeoutMsec - Timeout period to wait for request completion in mSec.
        //
        //  Return Value:
        //      ERROR_SUCCESS,
        //      ERROR_INVALID_HANDLE: Invalid request handle.
        //      ERROR_TIMEWOUT: Timeout period has expired.
        //      ERROR_REQUEST_ABORTED: Request was canceled, or wait was aborted.
        //
        uint32 Wait(uint32 TimeoutMsec);

        //
        //  Routine Description:
        //      Cancel is called by the  Bridge component to cancel a pending
        //      request.
        //
        //  Arguments:
        //
        //  Return Value:
        //      ERROR_SUCCESS,
        //      ERROR_INVALID_HANDLE: Invalid request handle.
        //      ERROR_NOT_CAPABLE: Cannot cancel the request at this point.
        //
        uint32 Cancel();

        //
        //  Routine Description:
        //      Release is called by the Bridge component to release a request.
        //
        //  Arguments:
        //
        //  Return Value:
        //      ERROR_SUCCESS,
        //      ERROR_INVALID_HANDLE: Invalid request handle.
        //
        uint32 Release();
    };


    //
    // ENUM_DEVICES_OPTIONS EnumDevice options.
    // Description:
    //  CACHE_ONLY - Get cached device list.
    //  FORCE_REFRESH - Re-enumerate device list.
    //
    public enum class ENUM_DEVICES_OPTIONS
    {
        CACHE_ONLY = 0,
        FORCE_REFRESH
    };


    //
    // IAdapter interface class.
    // Description:
    //  A pure virtual calls that defines the DSB Adapter interface.
    //  All current and future DSB Adapters implement this interface
    //
    public interface class IAdapter
    {
    public:

        //
        // Adapter information:
        //
        property Platform::String^ Vendor
        {
            Platform::String^ get();
        }
        property Platform::String^ AdapterName
        {
            Platform::String^ get();
        }
        property Platform::String^ Version
        {
            Platform::String^ get();
        }
        property Platform::String^ ExposedAdapterPrefix
        {
            Platform::String^ get();
        }
        property Platform::String^ ExposedApplicationName
        {
            Platform::String^ get();
        }
        property Platform::Guid ExposedApplicationGuid
        {
            Platform::Guid get();
        }

        // Adapter signals
        property IAdapterSignalVector^ Signals
        {
            IAdapterSignalVector^ get();
        }

        //
        //  Routine Description:
        //      SetConfiguration is called by the Bridge component to set the adapter configuration.
        //
        //  Arguments:
        //
        //      ConfigurationData - byte array that holds the new adapter configuration.
        //
        //  Return Value:
        //      ERROR_SUCCESS,
        //      ERROR_BAD_CONFIGURATION: Bad configuration.
        //      ERROR_SUCCESS_RESTART_REQUIRED: Changes were successfully updated, but will
        //          take affect after adapter has been restarted.
        //      ERROR_SUCCESS_REBOOT_REQUIRED:Changes were successfully updated, but will
        //          take affect after machine has been rebooted.
        //
        uint32 SetConfiguration(_In_ const Platform::Array<byte>^ ConfigurationData);

        //
        //  Routine Description:
        //      GetConfiguration is called by the Bridge component to get the current adapter configuration.
        //
        //  Arguments:
        //
        //      ConfigurationDataPtr - Address of an Array var to receive the current
        //          adapter configuration.
        //
        //  Return Value:
        //      ERROR_SUCCESS,
        //      ERROR_NOT_ENOUGH_MEMORY : Not enough resources to completed the operation.
        //
        uint32 GetConfiguration(_Out_ Platform::Array<byte>^* ConfigurationDataPtr);

        //
        //  Routine Description:
        //      Initialize is called by the Bridge component to initialize the adapter.
        //
        //  Arguments:
        //
        //
        //  Return Value:
        //      ERROR_SUCCESS, or error code specific to the cause the failed the
        //          initialization process.
        //
        uint32 Initialize();

        //
        //  Routine Description:
        //      Shutdown is called by the Bridge component to shutdown the adapter.
        //      All adapter resources are freed.
        //
        //  Arguments:
        //
        //
        //  Return Value:
        //      ERROR_SUCCESS, or error code specific to the cause the failed the
        //          shutdown process.
        //
        uint32 Shutdown();

        //
        //  Routine Description:
        //      EnumDevices is called by the Bridge component to get/enumerate adapter device list.
        //      The caller can either get the cached list, which the adapter maintains, or force the
        //      adapter to refresh its device list.
        //
        //  Arguments:
        //
        //      Options - Please refer to ENUM_DEVICES_OPTIONS supported values.
        //
        //      DeviceListPtr - Reference to a caller IAdapterDeviceVector^ car to
        //          receive the list of devices references.
        //
        //      RequestPtr - Reference to a caller IAdapterIoRequest^ variable to receive the
        //          request object, or nullptr for synchronous IO.
        //          If the request cannot be completed immediately, the caller can wait for
        //          its completion using WaitRequest() method, or cancel it using CancelRequest().
        //
        //  Return Value:
        //      ERROR_SUCCESS,
        //      ERROR_IO_PENDING: The request is still in progress.
        //      ERROR_NOT_ENOUGH_MEMORY : Not enough resources to complete the operation,
        //      or other error code related to the underlying stack.
        //
        uint32 EnumDevices(
            _In_ ENUM_DEVICES_OPTIONS Options,
            _Out_ IAdapterDeviceVector^* DeviceListPtr,
            _Out_opt_ IAdapterIoRequest^* RequestPtr
            );

        //
        //  Routine Description:
        //      GetProperty is called by the Bridge component to re-read an adapter device property.
        //
        //  Arguments:
        //
        //      Property - The target property, that the caller
        //          has previously acquired by using EnumDevices().
        //          The entire property (information and attributes) is re-read from the
        //          from the device.
        //
        //      RequestPtr - Reference to a caller IAdapterIoRequest^ variable to receive the
        //          request object, or nullptr for synchronous IO.
        //          If the request cannot be completed immediately, the caller can wait for
        //          its completion using WaitRequest() method, or cancel it using CancelRequest().
        //
        //  Return Value:
        //      ERROR_SUCCESS,
        //      ERROR_IO_PENDING: The request is still in progress.
        //      ERROR_INVALID_HANDLE: Invalid device/property handle.
        //      ERROR_NOT_ENOUGH_MEMORY : Not enough resources to completed the operation,
        //      or other error code related to the underlying stack.
        //
        uint32 GetProperty(
            _Inout_ IAdapterProperty^ Property,
            _Out_opt_ IAdapterIoRequest^* RequestPtr
            );

        //
        //  Routine Description:
        //      SetProperty is called by the Bridge component to set/write an adapter device property.
        //
        //  Arguments:
        //
        //      Property- The target property, that the caller
        //          has previously acquired by using EnumDevices().
        //
        //      ValueString - Property value in string format
        //
        //      RequestPtr - Reference to a caller IAdapterIoRequest^ variable to receive the
        //          request object, or nullptr for synchronous IO.
        //          If the request cannot be completed immediately, the caller can wait for
        //          its completion using WaitRequest() method, or cancel it using CancelRequest().
        //
        //  Return Value:
        //      ERROR_SUCCESS,
        //      ERROR_IO_PENDING: The request is still in progress.
        //      ERROR_INVALID_HANDLE: Invalid device/property handle.
        //      ERROR_NOT_ENOUGH_MEMORY : Not enough resources to completed the operation,
        //      or other error code related to the underlying stack.
        //
        uint32 SetProperty(
            _In_ IAdapterProperty^ Property,
            _Out_opt_ IAdapterIoRequest^* RequestPtr
            );

        //
        //  Routine Description:
        //      GetPropertyValue is called by the Bridge component to read a specific value of
        //      an adapter device property.
        //
        //  Arguments:
        //
        //      PropertyHandle - The desired property handle, that the caller
        //          has previously acquired by using EnumDevices().
        //
        //      AttributeName - The name for the desired attribute (IAdapterValue) of the Property.
        //
        //      ValuePtr - Reference to a caller IAdapterValue to receive the
        //          current value from the device.
        //
        //      RequestPtr - Reference to a caller IAdapterIoRequest^ variable to receive the
        //          request object, or nullptr for synchronous IO.
        //          If the request cannot be completed immediately, the caller can wait for
        //          its completion using WaitRequest() method, or cancel it using CancelRequest().
        //
        //  Return Value:
        //      ERROR_SUCCESS,
        //      ERROR_IO_PENDING: The request is still in progress.
        //      ERROR_INVALID_HANDLE: Invalid property handle.
        //      ERROR_NOT_ENOUGH_MEMORY : Not enough resources to completed the operation,
        //      or other error code related to the underlying stack.
        //
        uint32 GetPropertyValue(
            _In_ IAdapterProperty^ Property,
            _In_ Platform::String^ AttributeName,
            _Out_ IAdapterValue^* ValuePtr,
            _Out_opt_ IAdapterIoRequest^* RequestPtr
            );

        //
        //  Routine Description:
        //      SetPropertyValue is called by the Bridge component to write a specific value of
        //      an adapter device property.
        //
        //  Arguments:
        //
        //      Property - The target property, that the caller
        //          has previously acquired by using EnumDevices().
        //
        //      Value - The value to be set.
        //
        //      RequestPtr - Reference to a caller IAdapterIoRequest^ variable to receive the
        //          request object, or nullptr for synchronous IO.
        //          If the request cannot be completed immediately, the caller can wait for
        //          its completion using WaitRequest() method, or cancel it using CancelRequest().
        //
        //  Return Value:
        //      ERROR_SUCCESS,
        //      ERROR_IO_PENDING: The request is still in progress.
        //      ERROR_INVALID_HANDLE: Invalid device/property handle.
        //      ERROR_INVALID_PARAMETER: Invalid adapter property value information.
        //      ERROR_NOT_ENOUGH_MEMORY : Not enough resources to completed the operation,
        //      or other error code related to the underlying stack.
        //
        uint32 SetPropertyValue(
            _In_ IAdapterProperty^ Property,
            _In_ IAdapterValue^ Value,
            _Out_opt_ IAdapterIoRequest^* RequestPtr
            );

        //
        //  Routine Description:
        //      CallMethod is called by the Bridge component to call a device method
        //
        //  Arguments:
        //
        //      Method - The method to call.
        //          Caller needs to set the input parameters before calling CallMethod().
        //
        //      RequestPtr - Address of a caller allocated IAdapterIoRequest variable to receive the
        //          request handle, or nullptr for synchronous IO.
        //          If the request cannot be completed immediately, the caller can wait for
        //          its completion using WaitRequest() method, or cancel it using CancelRequest().
        //
        //  Return Value:
        //      ERROR_SUCCESS,
        //      ERROR_IO_PENDING: The request is still in progress.
        //      ERROR_INVALID_HANDLE: Invalid device.
        //      ERROR_INVALID_PARAMETER: Invalid adapter method information.
        //      ERROR_NOT_ENOUGH_MEMORY : Not enough resources to completed the operation,
        //      or other error code related to the underlying stack.
        //
        uint32 CallMethod(
            _Inout_ IAdapterMethod^ Method,
            _Out_opt_ IAdapterIoRequest^* RequestPtr
            );

        //
        //  Routine Description:
        //      RegisterSignalListener is called by the Bridge component to register for
        //      intercepting device generated signals.
        //
        //  Arguments:
        //
        //      Signal - The device signal the caller wishes to listen to,
        //          that was previously acquired by using EnumDevices().
        //
        //      Listener - The listener object.
        //
        //      ListenerContext - An optional context that will be passed to the signal
        //          handler.
        //
        //  Return Value:
        //      ERROR_SUCCESS,
        //      ERROR_INVALID_HANDLE: Invalid device/signal handles.
        //      ERROR_NOT_ENOUGH_MEMORY : Not enough resources to completed the operation.
        //
        uint32 RegisterSignalListener(
            _In_ IAdapterSignal^ Signal,
            _In_ IAdapterSignalListener^ Listener,
            _In_opt_ Platform::Object^ ListenerContext
            );

        //
        //  Routine Description:
        //      UnregisterSignalListener is called by the Bridge component to unregister from
        //      intercepting device generated signals.
        //
        //  Arguments:
        //
        //      Signal - The signal the caller wishes to stop listening to.
        //
        //      ListenerPtr - The listener object.
        //
        //  Return Value:
        //      ERROR_SUCCESS,
        //      ERROR_INVALID_PARAMETER: The caller is not registered as a signal listener.
        //
        uint32 UnregisterSignalListener(
            _In_ IAdapterSignal^ Signal,
            _In_ IAdapterSignalListener^ Listener
            );
    };
}
