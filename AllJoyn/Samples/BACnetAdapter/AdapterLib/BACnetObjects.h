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

#ifndef _BACNET_OBJECTS_H_
#define _BACNET_OBJECTS_H_

#include "AdapterDefinitions.h"
#include "BACnetDef.h"

namespace AdapterLib
{

    // Last attribute mark
    #define BACNET_ADAPTER_LAST_ATTRIBUTE         BACNET_PROPERTY_ID(-1)

    // Last attribute mark
    #define BACNET_ADAPTER_LAST_SIGNAL            BACNET_ADAPTER_SIGNAL_TYPE(-1)

    // Max number of attributes per BACnet object
    #define BACNET_ADAPTER_MAX_OBJECT_ATTRIBUTES  30

    // Max number of signals per BACnet object
    #define BACNET_ADAPTER_MAX_OBJECT_SIGNALS     5

    //
    // COMMON_ATTRIBUTES.
    // Description:
    //      Common BACnet attribute indexes
    //
    enum COMMON_ATTRIBUTES
    {
        CommonAttribute_Object_Type = 0,
        CommonAttribute_Object_Identifer,

        CommonAttribute_COUNT
    };

    //
    // DSB_SIGNAL_TYPE.
    // Description:
    //      The various types of signals from the adapter.
    //
    enum BACNET_ADAPTER_SIGNAL_TYPE
    {
        BACnetAdapterSignalTypeDeviceInvalid = 0,
        BACnetAdapterSignalTypeDeviceArrival,
        BACnetAdapterSignalTypeValueChanged
    };

    //
    // BACNET_ADAPTER_DEVICE_ARRIVAL_SIGNAL_PARAMETERS.
    // Description:
    //      'Device Arrival' signal parameters indexes
    //
    enum BACNET_ADAPTER_DEVICE_ARRIVAL_SIGNAL_PARAMETERS
    {
        DeviceArrivalSignal_DeviceHandle = 0,

        DeviceArrivalSignal_MAX
    };

    //
    // BACNET_ADAPTER_COV_SIGNAL_PARAMETERS.
    // Description:
    //      'Change Of Value' signal parameters indexes
    //
    enum BACNET_ADAPTER_COV_SIGNAL_PARAMETERS
    {
        CovSignal_DeviceHandle = 0,
        CovSignal_PropertyHandle,
        CovSignal_ValueName,
        CovSignal_ValueData,

        CovSignal_MAX
    };

    //
    // BACNET_ADAPTER_OBJECT_ID
    //
    struct BACNET_ADAPTER_OBJECT_ID
    {
        BACNET_ADAPTER_OBJECT_ID(ULONG ObjectType, ULONG ObjectInstance)
        {
            this->Bits.Instance = unsigned(ObjectInstance);
            this->Bits.Type = unsigned(ObjectType);
        }

        BACNET_ADAPTER_OBJECT_ID(ULONG ObjectId)
        {
            this->Ulong  = ObjectId;
        }

        BACNET_ADAPTER_OBJECT_ID()
        {
            this->Ulong = ULONG(-1);
        }

        union
        {
            struct
            {
                // Object type (refer to BACnet BACNET_OBJECT_TYPE)
                unsigned Type     : 10;

                // Object instance number
                unsigned Instance : 22;

            } Bits;

            ULONG Ulong;
        };
    };


    //
    // BACNET_ADAPTER_ATTRIBUTE_DESCRIPTOR
    // Description:
    //      A single property attribute descriptor,
    //      describes how a BACnet property attribute is
    //      shown in the DSB name space.
    //
    struct BACNET_ADAPTER_ATTRIBUTE_DESCRIPTOR
    {
        //
        // Attribute ID, used as the index
        // into the property attribute array.
        //
        BACNET_PROPERTY_ID      Id;

        // The attribute signature
        BridgeRT::AdapterValueType  Signature;

        // The BACnet signature for this type
        BACNET_APPLICATION_TAG  BACnetSignature;

        // Do we need to read it from device?
        bool                    IsReadFromDevice;

        //Access
        BridgeRT::E_ACCESS_TYPE Access;

        //COV Signal Behavior
        BridgeRT::SignalBehavior COVBehavior;
    };

    //
    // BACNET_ADAPTER_SIGNAL_COV
    //
    struct BACNET_ADAPTER_SIGNAL_COV
    {
        // BACNET property Id to subscribe to
        BACNET_PROPERTY_ID      PropId;

        // The runtime BACNET host object Id
        ULONG                   ObjectId;
    };

    //
    // BACNET_ADAPTER_SIGNAL_DESCRIPTOR
    //
    struct BACNET_ADAPTER_SIGNAL_DESCRIPTOR
    {
        // The Signal type
        BACNET_ADAPTER_SIGNAL_TYPE     Type;

        // Per signal type parameters
        union
        {
            BACNET_ADAPTER_SIGNAL_COV Cov;
        };
    };

    //
    // BACNET_ADAPTER_OBJECT_DESCRIPTOR structure.
    // Description:
    //      BACNET objects representation in DSB objects
    //
    struct BACNET_ADAPTER_OBJECT_DESCRIPTOR
    {
        // BACnet object type
        BACNET_OBJECT_TYPE              ObjectType;

        // BACnet object attribute descriptors
        BACNET_ADAPTER_ATTRIBUTE_DESCRIPTOR AttributeDescriptors[BACNET_ADAPTER_MAX_OBJECT_ATTRIBUTES];

        // BACnet object signals descriptors
        BACNET_ADAPTER_SIGNAL_DESCRIPTOR    SignalDescriptors[BACNET_ADAPTER_MAX_OBJECT_SIGNALS];
    };


    //
    //  Routine Description:
    //      GetObjetDescriptor() gets the BACnet object descriptor
    //      given the BACNET object type.
    //      The caller then uses the descriptor to build a new DSB object based
    //      on the BACnet object attributes (properties).
    //
    //  Arguments:
    //
    //      BACnetObjectType - The BACNET object type
    //
    //  Return Value:
    //      The address of the matching BACNET_ADAPTER_OBJECT_DESCRIPTOR or nullptr,
    //      if the give BACnet object is not supported.
    //
    const BACNET_ADAPTER_OBJECT_DESCRIPTOR*
    GetObjectDescriptor(
        BACNET_OBJECT_TYPE BACnetObjectType
        );


    //
    //  Routine Description:
    //      GetObjectAttributeDescriptorIndex() gets the attribute descriptor
    //      index given the BACNET object descriptor and the BACnet property ID.
    //      The routine searches through the list of attributes the given object
    //      has and returns the attribute descriptor that corresponds to the given
    //      property ID.
    //
    //  Arguments:
    //
    //      BACnetObjectDescriptorPtr - The property descriptor address.
    //
    //      BACNetPropertyId - The BACnet property ID to look for.
    //
    //  Return Value:
    //      The index of the matching BACNET_ADAPTER_ATTRIBUTE_DESCRIPTOR or -1,
    //      if the give BACnet property does not have the desired property as one of
    //      its attributes.
    //
    int
    GetObjectAttributeDescriptorIndex(
        _In_ const BACNET_ADAPTER_OBJECT_DESCRIPTOR* BACnetObjectDescriptorPtr,
        _In_ BACNET_PROPERTY_ID BACNetPropertyId
        );

    //
    //  Routine Description:
    //      GetObjectAttributeDescriptor() gets the attribute descriptor
    //      given the BACNET object ID and the BACnet property ID.
    //
    //  Arguments:
    //
    //      BACnetObjectType - The BACnet object type.
    //
    //      BACnetPropertyId - The BACnet property ID to look for.
    //
    //  Return Value:
    //      The address of the of BACNET_ADAPTER_ATTRIBUTE_DESCRIPTOR or nullptr,
    //      if the give BACnet property does not have the desired property as one of
    //      its attributes.
    //
    const BACNET_ADAPTER_ATTRIBUTE_DESCRIPTOR*
    GetObjectAttributeDescriptor(
        BACNET_OBJECT_TYPE BACnetObjectType,
        BACNET_PROPERTY_ID BACnetPropertyId
        );

    //
    // BACnet objects string conversions
    //
    const wchar_t* ToString(BACNET_PROPERTY_ID BACnetPropId);
    const wchar_t* ToString(BACNET_OBJECT_TYPE BACnetObjectType);
    const wchar_t* ToString(BACNET_ENGINEERING_UNITS BACnetUnits);
    const wchar_t* ToString(BACNET_POLARITY BACnetPolarity);
    const wchar_t* ToString(BACNET_ADAPTER_SIGNAL_TYPE signalType);

    DWORD FromString(_In_z_ const wchar_t* BACnetObjectTypeWsz, _Out_ BACNET_OBJECT_TYPE* BACnetObjectTypePtr);
    DWORD FromString(_In_z_ const wchar_t* BACnetObjectPropertyWsz, _Out_ BACNET_PROPERTY_ID* BACnetObjectPropertyIdPtr);
    DWORD FromString(_In_z_ const wchar_t* BACnetUnitsWsz, _Out_ BACNET_ENGINEERING_UNITS* BACnetUnitsPtr);
    DWORD FromString(_In_z_ const wchar_t* BACnetPolarityWsz, _Out_ BACNET_POLARITY* BACnetPolarityPtr);


    //
    //  Routine Description:
    //      GetWin32Code() gets the Win32 error code, given the BACnet
    //      error code.
    //
    //      Note:
    //          Not all BACnet codes are translated, if an error code
    //          is not translated, ERROR_GEN_FAILURE is returned.
    //
    //  Arguments:
    //
    //      BACnetErrorCode - The BACnet error code.
    //
    //  Return Value:
    //      The matching Win32 error code, or ERROR_GEN_FAILURE if
    //      error code is not translated.
    //
    DWORD
    GetWin32Code(
        BACNET_ERROR_CODE BACnetErrorCode
        );

} // namespace AdapterLib

#endif // !_BACNET_OBJECTS_H_