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
#include "BACnetObjects.h"


namespace AdapterLib
{
    //
    // BACnetAdapterValue.
    // Description:
    //  The class that implements BridgeRT::IAdapterValue.
    //
    ref class BACnetAdapterValue : BridgeRT::IAdapterValue
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

    internal:
        BACnetAdapterValue(
            Platform::String^ ObjectName,
            Platform::Object^ ParentObject,
            Platform::Object^ DefaultData = nullptr // For signature initialization
            );
        BACnetAdapterValue(const BACnetAdapterValue^ Other);

        uint32 Set(BridgeRT::IAdapterValue^ Other);
        uint32 Set(_In_ const BACNET_APPLICATION_DATA_VALUE& BACnetValue);

        uint32 FromBACnet(
            _In_ const BACNET_APPLICATION_DATA_VALUE& BACnetValue
            );
        uint32 ToBACnet(
            _In_ BACNET_ADAPTER_OBJECT_ID& BACnetObjectId,
            _In_ BACNET_PROPERTY_ID BACnetPropertyId,
            _Out_ BACNET_APPLICATION_DATA_VALUE& BACnetValue
            );

        static uint32 TranslateAttributeValue(
            _Inout_ BACnetAdapterValue^ Attribute,
            _In_ ULONG BACnetPropertyId,
            _In_ bool IsFromBACnet
            );

        bool IsModified() const { return this->isModified; }
        void SetModified(bool IsModified) { this->isModified = IsModified; }

    private:
        // Generic
        Platform::String^ name;
        Platform::Object^ parent;

        Platform::Object^ data;

        bool isModified;
    };

    //Forward declaration
    ref class BACnetAdapterAttribute;

    //
    // BACnetAdapterProperty.
    // Description:
    //  The class that implements BridgeRT::IAdapterProperty.
    //
    ref class BACnetAdapterProperty : BridgeRT::IAdapterProperty
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

        virtual property Platform::Object^ Parent
        {
            Platform::Object^ get() { return this->parent; }
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
        BACnetAdapterProperty(Platform::String^ Name, Platform::Object^ ParentObject, Platform::String^ ifHint = L"");
        BACnetAdapterProperty(ULONG BACnetObjectId, Platform::Object^ ParentObject, Platform::String^ ifHint = L"");
        BACnetAdapterProperty(const BACnetAdapterProperty^ Other);

        uint32 Set(BridgeRT::IAdapterProperty^ Other);

        void SetName(Platform::String^ PropertyName);

        ULONG GetBACnetObjectId();

        // Adding attributes
        BACnetAdapterProperty^ operator += (BridgeRT::IAdapterAttribute^ Attribute)
        {
            this->attributes.push_back(Attribute);
            return this;
        }

        BridgeRT::IAdapterValue^ GetPresentValue();
        uint32 SetPresentValue(const BACNET_APPLICATION_DATA_VALUE& PresetValue);

        BACnetAdapterAttribute^ GetAttributeByName(Platform::String^ AttributeName);
        BACnetAdapterAttribute^ GetAttributeByPropertyId(BACNET_PROPERTY_ID PropertyId);
        uint32 SetAttributeByPropertyId(_In_ BACNET_PROPERTY_ID PropertyId, _In_ const BACNET_APPLICATION_DATA_VALUE& Value);

        void NotifyCovSignal();

        void Relinquish(bool IsOnlyModified);

    private:
        // Generic
        Platform::String^ name;
        Platform::String^ interfaceHint;
        Platform::Object^ parent;

        std::vector<BridgeRT::IAdapterAttribute^> attributes;

        // The BACnet object ID
        ULONG objectId;
    };

    //
    // BACnetAdapterAttribute.
    // Description:
    //  The class that implements BridgeRT::IAdapterAttribute.
    //
    ref class BACnetAdapterAttribute : BridgeRT::IAdapterAttribute
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
        BACnetAdapterAttribute(
            Platform::String^ ObjectName,
            Platform::Object^ ParentObject,
            Platform::Object^ DefaultData = nullptr // For signature initialization
            );
        BACnetAdapterAttribute(const BACnetAdapterAttribute^ Other);

    private:
        // Generic
        BACnetAdapterValue^ value;

        std::map<Platform::String^, Platform::String^> annotations;
        BridgeRT::E_ACCESS_TYPE access = BridgeRT::E_ACCESS_TYPE::ACCESS_READ;    // By default - Read access only
        BridgeRT::SignalBehavior covBehavior = BridgeRT::SignalBehavior::Never;
    };


    //
    // BACnetAdapterSignal.
    // Description:
    //  The class that implements BridgeRT::IAdapterSignal.
    //
    ref class BACnetAdapterSignal : BridgeRT::IAdapterSignal
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
        BACnetAdapterSignal(Platform::String^ ObjectName, Platform::Object^ ParentObject)
            : name(ObjectName)
            , parent(ParentObject)
        {
        }

        // Adding parameters
        BACnetAdapterSignal^ operator += (BridgeRT::IAdapterValue^ Parameter)
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
    // SIGNAL_SUBSCRIPTION.
    // Description:
    //      Keeps all information regarding a single signal subscription,
    //      so the device can subscribe/un-subscribe.
    //
    //  Note:
    //      A device keeps a single AdapterSignal object per signal, which can
    //      be associated with multiple objects. For example a COV device AdapterSignal object
    //      is associated with all devices objects that have 'Present_Value' attribute.
    //
    struct SIGNAL_SUBSCRIPTION
    {
        // The signal type and parameters
        BACNET_ADAPTER_SIGNAL_DESCRIPTOR SignalDescriptor;

        // If subscription is active
        bool IsActive;

        SIGNAL_SUBSCRIPTION()
            : IsActive(false)
        {
            this->SignalDescriptor.Type = BACnetAdapterSignalTypeDeviceInvalid;
            RtlFillMemory(&this->SignalDescriptor, sizeof(this->SignalDescriptor), 0xFF);
        }
    };


    //
    // BACnetAdapterDevice.
    // Description:
    //  The class that implements BridgeRT::IAdapterDevice.
    //
    ref class BACnetAdapter;
    ref class BACnetInterface;
    ref class BACnetAdapterDevice : BridgeRT::IAdapterDevice,
                                    BridgeRT::IAdapterDeviceLightingService,
                                    BridgeRT::IAdapterDeviceControlPanel
    {
    public:
        //
        // Generic for Adapter objects
        //
        virtual property Platform::String^ Name
        {
            Platform::String^ get() { return this->name; }
        }
        virtual property Platform::Object^ Parent
        {
            Platform::Object^ get() { return this->parent; }
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
            BridgeRT::IAdapterMethodVector^ get() { return nullptr; }
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
                return nullptr;
            }
        }

        // Lighting Service Handler
        virtual property BridgeRT::ILSFHandler^ LightingServiceHandler
        {
            BridgeRT::ILSFHandler^ get()
            {
                return nullptr;
            }
        }

        // About Icon
        virtual property BridgeRT::IAdapterIcon^ Icon
        {
            BridgeRT::IAdapterIcon^ get()
            {
                return nullptr;
            }
        }

    internal:
        BACnetAdapterDevice(Platform::String^ Name, Platform::Object^ ParentObject);
        BACnetAdapterDevice(Platform::Object^ ParentObject);

        uint32 Init(
                UINT32 DeviceId,
                UINT16 VendorId,
                BACnetInterface^ StackInterface
                );
        void Shutdown();

        BACnetAdapterProperty^ GetPropertyByObjectId(ULONG PropertyObjectId);

        uint32 ReadProperty(
                _In_ ULONG PropertyObjectId,
                _Inout_ BridgeRT::IAdapterProperty^ adapterProperty,
                _Out_opt_ BridgeRT::IAdapterIoRequest^* RequestPtr
                );

        uint32 ReadPropertyAttribute(
                _Inout_ BridgeRT::IAdapterProperty^ adapterProperty,
                _Inout_ BridgeRT::IAdapterValue^* adapterAttributePtr,
                _Out_opt_ BridgeRT::IAdapterIoRequest^* RequestPtr
                );

        uint32 WritePropertyAttribute(
                _Inout_ BridgeRT::IAdapterProperty^ adapterProperty,
                _In_ BridgeRT::IAdapterValue^ adapterAttribute,
                _In_ bool IsRelinquish,
                _Out_opt_ BridgeRT::IAdapterIoRequest^* RequestPtr
                );

        uint32 Subscribe(BridgeRT::IAdapterSignal^ adapterSignal, bool IsSubscribe);
        void UnsubscribeAll();

        void SendSignal(
                _In_ Platform::String^ SignalName,
                _In_opt_ BridgeRT::IAdapterProperty^ Property,
                _In_opt_ BridgeRT::IAdapterValue^ Attribute
                );
    private:
        uint32 readDeviceInformation();
        uint32 readObjectList();
        uint32 readObjects();

        uint32 subscribeForSignals();

        uint32 storeAttribute(
                BACnetAdapterValue^ DeviceAttribute,
                ULONG BACnetPropertyId
                );

        uint32 addSignalSubscription(
                _In_ const BACNET_ADAPTER_SIGNAL_DESCRIPTOR& SignalDescriptor,
                _In_ const BACNET_ADAPTER_OBJECT_ID& ObjectId
                );

        bool IsCovSupported(_In_ SIGNAL_SUBSCRIPTION &newSubscription, _In_ BACnetAdapterSignal^ signalOfType);
        uint32 buildCovSignal(_Inout_ BACnetAdapterSignal^* CovSignalPtr);

        uint32 doSubscribe(int SignalHashCode, bool IsSubscribe);

    private:
        // Generic
        Platform::String^ name;
        Platform::Object^ parent;

        // Device information
        Platform::String^ vendor;
        Platform::String^ model;
        Platform::String^ version;
        Platform::String^ firmwareVersion;
        Platform::String^ serialNumber;
        Platform::String^ description;

        // Sync object
        DsbCommon::CSLock lock;

        // The BACnet adapter
        BACnetAdapter^ bacnetAdapter;

        // Device properties
        std::vector<BridgeRT::IAdapterProperty^> properties;

        // Device signals
        std::vector<BridgeRT::IAdapterSignal^> signals;

        // The BACnet stack interface
        BACnetInterface^ stackInterface;

        // The BACnet device identifier
        UINT32 deviceId;

        // The vendor identifier
        UINT16 vendorId;

        // The associated objects device identifiers
        std::vector<ULONG> objectIdentifiers;

        // BACnet object id -> BACnetAdapterProperty
        std::map<ULONG, BACnetAdapterProperty^> propertyMap;

        // Signal type -> AdapterSignal
        std::map<BACNET_ADAPTER_SIGNAL_TYPE, BACnetAdapterSignal^> signalMap;

        // Signal subscriptions
        std::multimap<int, SIGNAL_SUBSCRIPTION> signalSubscriptions;
    };

} // namespace AdapterLib