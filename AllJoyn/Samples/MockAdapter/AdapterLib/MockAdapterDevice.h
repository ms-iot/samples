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
#include "MockDevices.h"
#include "MockAdapter.h"

namespace AdapterLib
{
    //Forward declaration
    ref class MockAdapterAttribute;

    //
    // MockAdapterValue.
    // Description:
    //  The class that implements BridgeRT::IAdapterValue.
    //
    ref class MockAdapterValue : BridgeRT::IAdapterValue
    {
    public:
        //
        // Generic for adapter objects
        //
        virtual property Platform::String^ Name
        {
            Platform::String^ get() { return this->name; }
        }
        virtual property Platform::Object^ Parent
        {
            Platform::Object^ get() { return this->parent; }
        }

        // Data
        virtual property Platform::Object^ Data
        {
            Platform::Object^ get() { return this->data; }
            void set(Platform::Object^ NewData) { this->data = NewData; }
        }

        uint32 Set(BridgeRT::IAdapterValue^ Other);

    internal:
        MockAdapterValue(
            Platform::String^ ObjectName,
            Platform::Object^ ParentObject,
            Platform::Object^ DefaultData = nullptr // For signature initialization
            );
        MockAdapterValue(const MockAdapterValue^ Other);

    private:
        // Generic
        Platform::String^ name;
        Platform::Object^ parent;

        Platform::Object^ data;
    };


    //
    // MockAdapterProperty.
    // Description:
    //  The class that implements BridgeRT::IAdapterProperty.
    //
    struct MOCK_PROPERTY_DESCRIPTOR;
    ref class MockAdapterProperty : BridgeRT::IAdapterProperty
    {
    public:
        //
        // Generic for adapter objects
        //
        virtual property Platform::String^ Name
        {
            Platform::String^ get() { return this->name; }
        }

        virtual property Platform::String^ InterfaceHint
        {
            Platform::String^ get() { return this->interfaceHint; }
        }

        virtual property BridgeRT::IAdapterDevice^ Parent
        {
            BridgeRT::IAdapterDevice^ get() { return this->parent; }
        }

        // Attributes
        virtual property BridgeRT::IAdapterAttributeVector^ Attributes
        {
            BridgeRT::IAdapterAttributeVector^ get()
            {
                return ref new BridgeRT::AdapterAttributeVector(this->attributes);
            }
        }

    internal:

        MockAdapterProperty(Platform::String^ Name, BridgeRT::IAdapterDevice^ ParentObject);
        MockAdapterProperty(const MOCK_PROPERTY_DESCRIPTOR* MockPropertyDescPtr, BridgeRT::IAdapterDevice^ ParentObject);
        MockAdapterProperty(const MockAdapterProperty^ Other);

        uint32 Set(BridgeRT::IAdapterProperty^ Other);

        MockAdapterAttribute^ GetAttributeByName(Platform::String^ Name);

        uint32 Reset();

        void NotifyCovSignal(BridgeRT::IAdapterValue^ Attribute);

    private:
        // Generic
        Platform::String^ name;
        Platform::String^ interfaceHint;
        BridgeRT::IAdapterDevice^ parent;


        const MOCK_PROPERTY_DESCRIPTOR* mockDescPtr;

        std::vector<BridgeRT::IAdapterAttribute^> attributes;
    };

    //
    // MockAdapterAttribute.
    // Description:
    //  The class that implements BridgeRT::IAdapterAttribute.
    //
    ref class MockAdapterAttribute : BridgeRT::IAdapterAttribute
    {
    public:

        //Value
        virtual property BridgeRT::IAdapterValue^ Value
        {
            BridgeRT::IAdapterValue^ get() { return this->value; }
        }

        // Annotations
        virtual property BridgeRT::IAnnotationMap^ Annotations
        {
            BridgeRT::IAnnotationMap^ get()
            {
                return ref new BridgeRT::AnnotationMap(this->annotations);
            }
        }

        // Access
        virtual property BridgeRT::E_ACCESS_TYPE Access
        {
            BridgeRT::E_ACCESS_TYPE get()
            {
                return this->access;
            }

            void set(BridgeRT::E_ACCESS_TYPE accessType)
            {
                this->access = accessType;
            }
        }
        
        //Change of Value signal supported
        virtual property BridgeRT::SignalBehavior COVBehavior
        {
            BridgeRT::SignalBehavior get() { return this->covBehavior; }

            void set(BridgeRT::SignalBehavior behavior)
            {
                this->covBehavior = behavior;
            }
        }

    internal:
        MockAdapterAttribute(
            Platform::String^ ObjectName,
            Platform::Object^ ParentObject,
            Platform::Object^ DefaultData = nullptr // For signature initialization
            );
        MockAdapterAttribute(const MockAdapterAttribute^ Other);
        
    private:
        // Generic
        MockAdapterValue^ value;

        std::map<Platform::String^, Platform::String^> annotations;
        BridgeRT::E_ACCESS_TYPE access = BridgeRT::E_ACCESS_TYPE::ACCESS_READ;    // By default - Read access only
        BridgeRT::SignalBehavior covBehavior = BridgeRT::SignalBehavior::Never;
    };


    //
    // MockAdapterMethod.
    // Description:
    //  The class that implements BridgeRT::IAdapterDevice.
    //
    ref class MockAdapterMethod :public BridgeRT::IAdapterMethod
    {
    public:
        // Object name
        virtual property Platform::String^ Name
        {
            Platform::String^ get() { return this->name; }
        }

        // Get the object's parent
        virtual property Platform::Object^ Parent
        {
            Platform::Object^ get() { return this->parent; }
        }

        // Method description
        virtual property Platform::String^ Description
        {
            Platform::String^ get() { return this->description; }
        }

        // The input parameters
        virtual property BridgeRT::IAdapterValueVector^ InputParams
        {
            BridgeRT::IAdapterValueVector^ get()
            {
                return ref new BridgeRT::AdapterValueVector(this->inParams);
            }
            void set(BridgeRT::IAdapterValueVector^ Params);
        }

        // The output parameters
        virtual property BridgeRT::IAdapterValueVector^ OutputParams
        {
            BridgeRT::IAdapterValueVector^ get()
            {
                return ref new BridgeRT::AdapterValueVector(this->outParams);
            }
        }

        // The return value
        virtual property int32 HResult
        {
            int32 get() { return this->result; }
        }

    internal:
        MockAdapterMethod(Platform::String^ ObjectName, Platform::Object^ ParentObject);
        MockAdapterMethod(const MockAdapterMethod^ Other);

        // Adding parameters
        void addInputParam(BridgeRT::IAdapterValue^ InParameter)
        {
            this->inParams.push_back(InParameter);
        }
        void addOutputParam(BridgeRT::IAdapterValue^ OutParameter)
        {
            this->outParams.push_back(OutParameter);
        }

        MockAdapterValue^ GetInputParamByName(Platform::String^ InputParamName);
        MockAdapterValue^ GetOutputParamByName(Platform::String^ OutputParamName);

        void setResult(HRESULT Hr);

    private:
        // Generic
        Platform::String^ name;
        Platform::Object^ parent;

        // Method information
        Platform::String^ description;

        // Method parameters
        std::vector<BridgeRT::IAdapterValue^> inParams;
        std::vector<BridgeRT::IAdapterValue^> outParams;
        int32 result;
    };


    //
    // MockAdapterSignal.
    // Description:
    //  The class that implements BridgeRT::IAdapterSignal.
    //
    ref class MockAdapterSignal : BridgeRT::IAdapterSignal
    {
    public:
        //
        // Generic for adapter objects
        //
        virtual property Platform::String^ Name
        {
            Platform::String^ get() { return this->name; }
        }
        virtual property Platform::Object^ Parent
        {
            Platform::Object^ get() { return this->parent; }
        }

        // Signal parameters
        virtual property BridgeRT::IAdapterValueVector^ Params
        {
            BridgeRT::IAdapterValueVector^ get()
            {
                return ref new BridgeRT::AdapterValueVector(this->params);
            }
        }

    internal:
        MockAdapterSignal(Platform::String^ ObjectName, Platform::Object^ ParentObject)
            : name(ObjectName)
            , parent(ParentObject)
        {
        }

        // Adding parameters
        MockAdapterSignal^ operator += (BridgeRT::IAdapterValue^ Parameter)
        {
            this->params.push_back(Parameter);
            return this;
        }

    private:
        // Generic
        Platform::String^ name;
        Platform::Object^ parent;

        std::vector<BridgeRT::IAdapterValue^> params;
    };


    //
    // MockAdapterIcon
    // Description:
    //  This class demonstrates how to provide an ICON for the Mock device
    //
    ref class MockAdapterIcon : BridgeRT::IAdapterIcon
    {
    public:

        // Gets the image data (Optional - but must set the URL if this returns null)
        virtual Platform::Array<BYTE>^ GetImage();

        // Gets the Icon's URL.  (Optional - but must set the Image Data if this returns null)
        virtual property Platform::String^ Url
        {
            Platform::String^ get()
            {
                return url;
            }
        }

        // Gets the Icon's type. (Returns the Mime Type of Image Data or URL. Must always be the same)
        virtual property Platform::String^ MimeType
        {
            Platform::String^ get()
            {
                return mimeType;
            }
        }


    internal:
        MockAdapterIcon(Windows::Storage::StorageFile^ srcFile, Platform::String^ srcUrl = nullptr);
        MockAdapterIcon(Platform::String^ srcUrl, Platform::String^ srcMimeType);

    private:
        Windows::Storage::StorageFile^ srcImageFile;
        Platform::String^ url;
        Platform::String^ mimeType;
    };


    //
    // MockAdapterDevice.
    // Description:
    //  The class that implements BridgeRT::IAdapterDevice.
    //
    struct MOCK_DEVICE_DESCRIPTOR;
    ref class MockAdapterDevice :   BridgeRT::IAdapterDevice, 
                                    BridgeRT::IAdapterDeviceLightingService, 
                                    BridgeRT::IAdapterDeviceControlPanel
    {
    public:
        //
        // Generic for adapter objects
        //
        virtual property Platform::String^ Name
        {
            Platform::String^ get() { return this->name; }
        }
        virtual property BridgeRT::IAdapter^ Parent
        {
            BridgeRT::IAdapter^ get() { return this->parent; }
        }

        //
        // Device information
        //
        virtual property Platform::String^ Vendor
        {
            Platform::String^ get() { return this->vendor; }
        }
        virtual property Platform::String^ Model
        {
            Platform::String^ get() { return this->model; }
        }
        virtual property Platform::String^ Version
        {
            Platform::String^ get() { return this->version; }
        }
        virtual property Platform::String^ FirmwareVersion
        {
            Platform::String^ get() { return this->firmwareVersion; }
        }
        virtual property Platform::String^ ID
        {
            Platform::String^ get() { return this->serialNumber; }
        }
        virtual property Platform::String^ SerialNumber
        {
            Platform::String^ get() { return this->serialNumber; }
        }
        virtual property Platform::String^ Description
        {
            Platform::String^ get() { return this->description; }
        }

        // Device properties
        virtual property BridgeRT::IAdapterPropertyVector^ Properties
        {
            BridgeRT::IAdapterPropertyVector^ get()
            {
                return ref new BridgeRT::AdapterPropertyVector(this->properties);
            }
        }

        // Device methods
        virtual property BridgeRT::IAdapterMethodVector^ Methods
        {
            BridgeRT::IAdapterMethodVector^ get();
        }

        // Device signals
        virtual property BridgeRT::IAdapterSignalVector^ Signals
        {
            BridgeRT::IAdapterSignalVector^ get()
            {
                return ref new BridgeRT::AdapterSignalVector(this->signals);
            }
        }

        // Control Panel Handler
        virtual property BridgeRT::IControlPanelHandler^ ControlPanelHandler
        {
            BridgeRT::IControlPanelHandler^ get()
            {
                return controlPanelHandler;
            }

            void set(BridgeRT::IControlPanelHandler^ handler)
            {
                controlPanelHandler = handler;
            }
        }

        // Lighting Service Handler
        virtual property BridgeRT::ILSFHandler^ LightingServiceHandler
        {
            BridgeRT::ILSFHandler^ get()
            {
                return lightingServiceHandler;
            }

            void set(BridgeRT::ILSFHandler^ handler)
            {
                lightingServiceHandler = handler;
            }
        }

        virtual property BridgeRT::IAdapterIcon^ Icon
        {
            BridgeRT::IAdapterIcon^ get()
            {
                return icon;
            }
        }

    internal:
        MockAdapterDevice(Platform::String^ Name, AdapterLib::MockAdapter^ ParentObject);
        MockAdapterDevice(const MOCK_DEVICE_DESCRIPTOR* MockDeviceDescPtr, MockAdapter^ ParentObject);

        void AddProperty(_In_ BridgeRT::IAdapterProperty^ NewProperty);
        void AddMethod(_In_ BridgeRT::IAdapterMethod^ NewMethod);
        void AddSignal(_In_ BridgeRT::IAdapterSignal^ NewSignal);

        void SendSignal(
            _In_ Platform::String^ SignalName,
            _In_opt_ BridgeRT::IAdapterProperty^ Property,
            _In_opt_ BridgeRT::IAdapterValue^ Attribute
            );

        uint32 MethodDispatch(
                _In_ BridgeRT::IAdapterMethod^ Method,
                _Out_opt_ BridgeRT::IAdapterIoRequest^* RequestPtr
                );

        MockAdapterProperty^ GetPropertyByName(_In_ Platform::String^ PropertyName);
        MockAdapterMethod^ GetMethodByName(_In_ Platform::String^ MethodName);
        MockAdapterSignal^ GetSignalByName(_In_ Platform::String^ SignalName);

        void SetVendor(Platform::String^ deviceVendor)
        {
            this->vendor = deviceVendor;
        }

        void SetModel(Platform::String^ deviceModel)
        {
            this->model = deviceModel;
        }

        void SetSerialNumber(Platform::String^ deviceSerialNumber)
        {
            this->serialNumber = deviceSerialNumber;
        }


    private:
        void createMethods();
        void createSignals();

        void methodReset(_Inout_ BridgeRT::IAdapterMethod^ Method);

    private:
        // Generic
        Platform::String^ name;
        MockAdapter^ parent;

        // Device information
        Platform::String^ vendor;
        Platform::String^ model;
        Platform::String^ version;
        Platform::String^ firmwareVersion;
        Platform::String^ serialNumber;
        Platform::String^ description;
        BridgeRT::IControlPanelHandler^ controlPanelHandler;
        BridgeRT::ILSFHandler^ lightingServiceHandler;
        BridgeRT::IAdapterIcon^ icon;

        // Sync object
        std::recursive_mutex lock;

        // Device properties
        std::vector<BridgeRT::IAdapterProperty^> properties;

        // Device methods
        std::vector<BridgeRT::IAdapterMethod^> methods;

        // Device signals
        std::vector<BridgeRT::IAdapterSignal^> signals;
    };

} // namespace AdapterLib