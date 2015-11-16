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
#include "AdapterDefinitions.h"
#include "BACnetDef.h"
#include "BACnetObjects.h"
#include "BfiDefinitions.h"
#include "Misc.h"

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;

using namespace BridgeRT;
using namespace DsbCommon;


namespace AdapterLib
{

    //
    // The BACnet -> DSB property descriptors
    //
    BACNET_ADAPTER_OBJECT_DESCRIPTOR BacnetAdapterObjectDescriptors[] =
    {
        // OBJECT_DEVICE
        {
            OBJECT_DEVICE,    // Object type

            // Attributes
            {
                {
                    PROP_OBJECT_TYPE,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    false,                                      // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_OBJECT_IDENTIFIER,                     // ID
                    PropertyType::UInt32,                       // Signature
                    BACNET_APPLICATION_TAG_OBJECT_ID,           // BACnet signature
                    false,                                      // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },

                {
                    PROP_OBJECT_NAME,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_VENDOR_NAME,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_DESCRIPTION,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READWRITE,            // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_MODEL_NAME,                            // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_FIRMWARE_REVISION,                     // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_OBJECT_LIST,                           // ID
                    PropertyType::UInt32Array,                  // Signature
                    BACNET_APPLICATION_TAG_OBJECT_ID,           // BACnet signature
                    false,                                      // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },

                // ...

                {BACNET_ADAPTER_LAST_ATTRIBUTE}

            }, // Attributes

            // Signals
            {
                // No signals at device descriptor level.
                {BACNET_ADAPTER_LAST_SIGNAL}

            } // Signals

        },

        // ANALOG_INPUT
        {
            OBJECT_ANALOG_INPUT,    // Object type

            // Attributes
            {
                {
                    PROP_OBJECT_TYPE,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    false,                                      // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_OBJECT_IDENTIFIER,                     // ID
                    PropertyType::UInt32,                       // Signature
                    BACNET_APPLICATION_TAG_OBJECT_ID,           // BACnet signature
                    false,                                      // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },

                {
                    PROP_OBJECT_NAME,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_DEVICE_TYPE,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READWRITE,            // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_DESCRIPTION,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READWRITE,            // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_PRESENT_VALUE,                         // ID
                    PropertyType::Double,                       // Signature
                    BACNET_APPLICATION_TAG_REAL,                // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Always                       // COV behavior
                },
                {
                    PROP_UNITS,                                 // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_ENUMERATED,          // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },

                // ...

                {BACNET_ADAPTER_LAST_ATTRIBUTE}

            }, // Attributes

            // Signals
            {
                {
                    BACnetAdapterSignalTypeValueChanged,
                    { PROP_PRESENT_VALUE }
                },

                {BACNET_ADAPTER_LAST_SIGNAL}

            } // Signals
        },

        // ANALOG_OUTPUT
        {
            OBJECT_ANALOG_OUTPUT,    // Object type

            // Attributes
            {
                {
                    PROP_OBJECT_TYPE,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    false,                                      // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_OBJECT_IDENTIFIER,                     // ID
                    PropertyType::UInt32,                       // Signature
                    BACNET_APPLICATION_TAG_OBJECT_ID,           // BACnet signature
                    false,                                      // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },

                {
                    PROP_OBJECT_NAME,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_DESCRIPTION,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READWRITE,            // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_PRESENT_VALUE,                         // ID
                    PropertyType::Double,                       // Signature
                    BACNET_APPLICATION_TAG_REAL,                // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READWRITE,            // Access
                    SignalBehavior::Always                       // COV behavior
                },
                {
                    PROP_UNITS,                                 // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_ENUMERATED,          // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },

                // ...

                {BACNET_ADAPTER_LAST_ATTRIBUTE}

            }, // Attributes

            // Signals
            {
                {
                    BACnetAdapterSignalTypeValueChanged,
                    { PROP_PRESENT_VALUE }
                },

                { BACNET_ADAPTER_LAST_SIGNAL }

            } // Signals
        },

        // BINARY_INPUT
        {
            OBJECT_BINARY_INPUT,    // Object type

            // Attributes
            {
                {
                    PROP_OBJECT_TYPE,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    false,                                      // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_OBJECT_IDENTIFIER,                     // ID
                    PropertyType::UInt32,                       // Signature
                    BACNET_APPLICATION_TAG_OBJECT_ID,           // BACnet signature
                    false,                                      // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },

                {
                    PROP_OBJECT_NAME,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_DESCRIPTION,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READWRITE,            // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_PRESENT_VALUE,                         // ID
                    PropertyType::Boolean,                      // Signature
                    BACNET_APPLICATION_TAG_ENUMERATED,          // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Always                       // COV behavior
                },
                {
                    PROP_POLARITY,                              // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_ENUMERATED,          // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },

                // ...

                {BACNET_ADAPTER_LAST_ATTRIBUTE}

            }, // Attributes

            // Signals
            {
                {
                    BACnetAdapterSignalTypeValueChanged,
                    { PROP_PRESENT_VALUE }
                },

                { BACNET_ADAPTER_LAST_SIGNAL }

            } // Signals
        },

        // BINARY_OUTPUT
        {
            OBJECT_BINARY_OUTPUT,    // Object type

            // Attributes
            {
                {
                    PROP_OBJECT_TYPE,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    false,                                      // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_OBJECT_IDENTIFIER,                     // ID
                    PropertyType::UInt32,                       // Signature
                    BACNET_APPLICATION_TAG_OBJECT_ID,           // BACnet signature
                    false,                                      // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },

                {
                    PROP_OBJECT_NAME,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_DESCRIPTION,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READWRITE,            // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_PRESENT_VALUE,                         // ID
                    PropertyType::Boolean,                      // Signature
                    BACNET_APPLICATION_TAG_ENUMERATED,          // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READWRITE,            // Access
                    SignalBehavior::Always                       // COV behavior
                },
                {
                    PROP_POLARITY,                              // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_ENUMERATED,          // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },

                {BACNET_ADAPTER_LAST_ATTRIBUTE}

            }, // Attributes

            // Signals
            {
                {
                    BACnetAdapterSignalTypeValueChanged,
                    { PROP_PRESENT_VALUE }
                },

                { BACNET_ADAPTER_LAST_SIGNAL }

            } // Signals
        },

        // ANALOG_VALUE
        {
            OBJECT_ANALOG_VALUE,   // Object type

            // Attributes
            {
                {
                    PROP_OBJECT_TYPE,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    false,                                      // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_OBJECT_IDENTIFIER,                     // ID
                    PropertyType::UInt32,                       // Signature
                    BACNET_APPLICATION_TAG_OBJECT_ID,           // BACnet signature
                    false,                                      // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },

                {
                    PROP_OBJECT_NAME,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_DEVICE_TYPE,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READWRITE,            // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_DESCRIPTION,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READWRITE,            // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_PRESENT_VALUE,                         // ID
                    PropertyType::Double,                       // Signature
                    BACNET_APPLICATION_TAG_REAL,                // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READWRITE,            // Access
                    SignalBehavior::Always                       // COV behavior
                },
                {
                    PROP_UNITS,                                 // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_ENUMERATED,          // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },

                // ...

                { BACNET_ADAPTER_LAST_ATTRIBUTE }

            }, // Attributes

            // Signals
            {
                {
                    BACnetAdapterSignalTypeValueChanged,
                    { PROP_PRESENT_VALUE }
                },

                { BACNET_ADAPTER_LAST_SIGNAL }

            } // Signals
        },

        // BINARY_VALUE
        {
            OBJECT_BINARY_VALUE,    // Object type

            // Attributes
            {
                {
                    PROP_OBJECT_TYPE,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    false,                                      // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_OBJECT_IDENTIFIER,                     // ID
                    PropertyType::UInt32,                       // Signature
                    BACNET_APPLICATION_TAG_OBJECT_ID,           // BACnet signature
                    false,                                      // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },

                {
                    PROP_OBJECT_NAME,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READ,                 // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_DESCRIPTION,                           // ID
                    PropertyType::String,                       // Signature
                    BACNET_APPLICATION_TAG_CHARACTER_STRING,    // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READWRITE,            // Access
                    SignalBehavior::Never                       // COV behavior
                },
                {
                    PROP_PRESENT_VALUE,                         // ID
                    PropertyType::Boolean,                      // Signature
                    BACNET_APPLICATION_TAG_ENUMERATED,          // BACnet signature
                    true,                                       // Read from device?
                    E_ACCESS_TYPE::ACCESS_READWRITE,            // Access
                    SignalBehavior::Always                       // COV behavior
                },

                { BACNET_ADAPTER_LAST_ATTRIBUTE }

            }, // Attributes

            // Signals
            {
                {
                    BACnetAdapterSignalTypeValueChanged,
                    { PROP_PRESENT_VALUE }
                },

                { BACNET_ADAPTER_LAST_SIGNAL }

            } // Signals
        },
    };


    //
    // DSB <-> BACnet helper functions implementation
    //

    const BACNET_ADAPTER_OBJECT_DESCRIPTOR*
    GetObjectDescriptor(
        BACNET_OBJECT_TYPE BACnetObjectType
        )
    {
        for (ULONG descInx = 0;  descInx < ARRAYSIZE(BacnetAdapterObjectDescriptors); ++descInx)
        {
            if (BacnetAdapterObjectDescriptors[descInx].ObjectType == BACnetObjectType)
            {
                return &BacnetAdapterObjectDescriptors[descInx];
            }
        }

        return nullptr;
    }


    _Use_decl_annotations_
    int
    GetObjectAttributeDescriptorIndex(
        const BACNET_ADAPTER_OBJECT_DESCRIPTOR* BACnetObjectDescriptorPtr,
        BACNET_PROPERTY_ID BACNetPropertyId
        )
    {
        for (ULONG descInx = 0;  descInx < ARRAYSIZE(BACnetObjectDescriptorPtr->AttributeDescriptors); ++descInx)
        {
            BACNET_PROPERTY_ID curId = BACnetObjectDescriptorPtr->AttributeDescriptors[descInx].Id;

            if (curId == BACNetPropertyId)
            {
                return descInx;
            }

            if (curId == BACNET_ADAPTER_LAST_ATTRIBUTE)
            {
                break;
            }
        }

        return -1;
    }


    const BACNET_ADAPTER_ATTRIBUTE_DESCRIPTOR*
    GetObjectAttributeDescriptor(
        BACNET_OBJECT_TYPE BACnetObjectType,
        BACNET_PROPERTY_ID BACnetPropertyId
        )
    {
        const BACNET_ADAPTER_OBJECT_DESCRIPTOR* objectDescPtr = GetObjectDescriptor(BACnetObjectType);
        if (objectDescPtr == nullptr)
        {
            return nullptr;
        }

        int attrDescIndex = GetObjectAttributeDescriptorIndex(objectDescPtr, BACnetPropertyId);
        if (attrDescIndex == -1)
        {
            return nullptr;
        }

        DSB_ASSERT(attrDescIndex < ARRAYSIZE(objectDescPtr->AttributeDescriptors));

        return &objectDescPtr->AttributeDescriptors[attrDescIndex];
    }


    //
    // BACNet Type mapping entry
    //
    struct BACnetTypeMapEntry
    {
        // Object type
		UINT32              ObjectType;

        // Object strings
        Platform::String^   ObjectString;
    };


    //
    // BACnet objects string conversion map
    //

    //
    // BACNET_PROPERTY_ID
    //
    static BACnetTypeMapEntry bacnetPropIdToStringMap[] =
    {
        { PROP_OBJECT_TYPE , L"Object_Type" },
        { PROP_OBJECT_NAME , L"Object_Name" },
        { PROP_DEVICE_TYPE , L"Device_Type" },
        { PROP_VENDOR_NAME , L"Vendor_Name" },
        { PROP_VENDOR_IDENTIFIER , L"Vendor_Identifier" },
        { PROP_MODEL_NAME , L"Model_Name" },
        { PROP_FIRMWARE_REVISION , L"Firmware_Revision" },
        { PROP_OBJECT_LIST , L"Object_list" },
        { PROP_OBJECT_IDENTIFIER, L"Object_Identifier" },
        { PROP_DESCRIPTION , L"Description" } ,
        { PROP_PRESENT_VALUE , L"Present_Value" },
        { PROP_UNITS , L"Units" },
        { PROP_PRIORITY_ARRAY , L"Prioriy_Array" },
        { PROP_RELINQUISH_DEFAULT , L"Relinquish_Default" },
        { PROP_POLARITY , L"Polarity" },
        { PROP_CHANGE_OF_STATE_TIME , L"Prioriy_Array" },
        { PROP_PRIORITY_ARRAY , L"State_Change_Time" },
        { PROP_MINIMUM_OFF_TIME , L"Min_Off_Time" },
        { PROP_MINIMUM_ON_TIME , L"Min_On_Time" },

        //...
    };


    //
    // BACNET_OBJECT_TYPE
    //
    static BACnetTypeMapEntry bacnetObjecTypeToStringMap[] =
    {
        { OBJECT_DEVICE , L"DEVICE" },
        { OBJECT_ANALOG_INPUT , L"ANALOG_INPUT" },
        { OBJECT_ANALOG_OUTPUT, L"ANALOG_OUTPUT" },
        { OBJECT_ANALOG_VALUE , L"ANALOG_VALUE" } ,
        { OBJECT_BINARY_INPUT , L"BINARY_INPUT" },
        { OBJECT_BINARY_OUTPUT , L"BINARY_OUTPUT" },
        { OBJECT_BINARY_VALUE , L"BINARY_VALUE" },

        //...
    };


    //
    // BACNET_ENGINEERING_UNITS
    //
    static BACnetTypeMapEntry bacnetUnitsToStringMap[] =
    {
        // Temperature
        { UNITS_DEGREES_CELSIUS , L"UNITS_DEGREES_CELSIUS" },
        { UNITS_DEGREES_KELVIN, L"UNITS_DEGREES_KELVIN" },
        { UNITS_DEGREES_KELVIN_PER_HOUR , L"UNITS_DEGREES_KELVIN_PER_HOUR" } ,
        { UNITS_DEGREES_KELVIN_PER_MINUTE , L"UNITS_DEGREES_KELVIN_PER_MINUTE" },
        { UNITS_DEGREES_FAHRENHEIT , L"UNITS_DEGREES_FAHRENHEIT" },
        { UNITS_DEGREE_DAYS_CELSIUS , L"UNITS_DEGREE_DAYS_CELSIUS" },
        { UNITS_DEGREE_DAYS_FAHRENHEIT , L"UNITS_DEGREE_DAYS_FAHRENHEIT" },
        { UNITS_DELTA_DEGREES_FAHRENHEIT , L"UNITS_DELTA_DEGREES_FAHRENHEIT" },
        { UNITS_DELTA_DEGREES_KELVIN , L"UNITS_DELTA_DEGREES_KELVIN" },

        // Time
        { UNITS_YEARS , L"UNITS_YEARS" },
        { UNITS_MONTHS, L"UNITS_MONTHS" },
        { UNITS_WEEKS , L"UNITS_WEEKS" } ,
        { UNITS_DAYS , L"UNITS_DAYS" },
        { UNITS_HOURS , L"UNITS_HOURS" },
        { UNITS_MINUTES , L"UNITS_MINUTES" },
        { UNITS_SECONDS , L"UNITS_SECONDS" },
        { UNITS_HUNDREDTHS_SECONDS , L"UNITS_HUNDREDTHS_SECONDS" },
        { UNITS_MILLISECONDS , L"UNITS_MILLISECONDS" },

        /* Other */
        { UNITS_NO_UNITS , L"UNITS_NO_UNITS" },
        { UNITS_PERCENT , L"UNITS_PERCENT" },

        //...
    };


    //
    // BACNET_POLARITY
    //
    static BACnetTypeMapEntry bacnetPolarityToStringMap[] =
    {
        { POLARITY_NORMAL , L"POLARITY_NORMAL" },
        { POLARITY_REVERSE, L"POLARITY_REVERSE" },

    }; // bacnetObjecToStringMap


    //
    // DSB_SIGNAL_TYPE
    //
    static BACnetTypeMapEntry bacnetAdapterSignalTypeToStringMap[] =
    {
        { BACnetAdapterSignalTypeDeviceArrival, BridgeRT::Constants::DEVICE_ARRIVAL_SIGNAL },
        { BACnetAdapterSignalTypeValueChanged, BridgeRT::Constants::CHANGE_OF_VALUE_SIGNAL },
    };


    const wchar_t*
    bacnetObjectToString(
        _In_ UINT32 BACnetObjectType,
        _In_reads_(MapArrayCount) BACnetTypeMapEntry MapArray[],
        _In_ size_t MapArrayCount
        )
    {
        for (UINT entryInx=0; entryInx < MapArrayCount; ++entryInx)
        {
            if (MapArray[entryInx].ObjectType == BACnetObjectType)
            {
                return MapArray[entryInx].ObjectString->Data();
            }
        }

        return nullptr;
    }


	UINT32
	bacnetObjectFromString(
		_In_z_ const wchar_t* BACnetObjectWsz,
		_In_reads_(MapArrayCount) BACnetTypeMapEntry MapArray[],
		_In_ size_t MapArrayCount
		)
	{
		for (UINT entryInx = 0; entryInx < MapArrayCount; ++entryInx)
		{
			if (_wcsicmp(BACnetObjectWsz, MapArray[entryInx].ObjectString->Data()) == 0)
			{
				return MapArray[entryInx].ObjectType;
			}
		}

		return UINT32(-1);
	}


    const wchar_t*
    ToString(BACNET_PROPERTY_ID BACnetPropId)
    {
        return bacnetObjectToString(UINT32(BACnetPropId), bacnetPropIdToStringMap, ARRAYSIZE(bacnetPropIdToStringMap));
    }


    const wchar_t*
    ToString(BACNET_OBJECT_TYPE BACnetObjectType)
    {
        return bacnetObjectToString(UINT32(BACnetObjectType), bacnetObjecTypeToStringMap, ARRAYSIZE(bacnetObjecTypeToStringMap));
    }


    const wchar_t*
    ToString(BACNET_ENGINEERING_UNITS BACnetUnits)
    {
        return bacnetObjectToString(UINT32(BACnetUnits), bacnetUnitsToStringMap, ARRAYSIZE(bacnetUnitsToStringMap));
    }


    const wchar_t*
    ToString(BACNET_POLARITY BACnetPolarity)
    {
        return bacnetObjectToString(UINT32(BACnetPolarity), bacnetPolarityToStringMap, ARRAYSIZE(bacnetPolarityToStringMap));
    }


    const wchar_t*
    ToString(BACNET_ADAPTER_SIGNAL_TYPE signalType)
    {
        return bacnetObjectToString(UINT32(signalType), bacnetAdapterSignalTypeToStringMap, ARRAYSIZE(bacnetAdapterSignalTypeToStringMap));
    }


    _Use_decl_annotations_
    DWORD
    FromString(const wchar_t* BACnetObjectTypeWsz, BACNET_OBJECT_TYPE* BACnetObjectTypePtr)
    {
        DSB_ASSERT(BACnetObjectTypePtr != nullptr);

        *BACnetObjectTypePtr = BACNET_OBJECT_TYPE(
            bacnetObjectFromString(BACnetObjectTypeWsz, bacnetObjecTypeToStringMap, ARRAYSIZE(bacnetObjecTypeToStringMap))
            );
        if (*BACnetObjectTypePtr == UINT32(-1))
        {
            return ERROR_NOT_SUPPORTED;
        }

        return ERROR_SUCCESS;
    }


    _Use_decl_annotations_
    DWORD
    FromString(const wchar_t* BACnetObjectPropertyWsz, BACNET_PROPERTY_ID* BACnetObjectPropertyIdPtr)
    {
        DSB_ASSERT(BACnetObjectPropertyIdPtr != nullptr);

        *BACnetObjectPropertyIdPtr = BACNET_PROPERTY_ID(
            bacnetObjectFromString(BACnetObjectPropertyWsz, bacnetPropIdToStringMap, ARRAYSIZE(bacnetPropIdToStringMap))
            );
        if (*BACnetObjectPropertyIdPtr == UINT32(-1))
        {
            return ERROR_NOT_SUPPORTED;
        }

        return ERROR_SUCCESS;
    }


	_Use_decl_annotations_
    DWORD
    FromString(const wchar_t* BACnetUnitsWsz, BACNET_ENGINEERING_UNITS* BACnetUnitsPtr)
    {
        DSB_ASSERT(BACnetUnitsPtr != nullptr);

        *BACnetUnitsPtr = BACNET_ENGINEERING_UNITS(
            bacnetObjectFromString(BACnetUnitsWsz, bacnetUnitsToStringMap, ARRAYSIZE(bacnetUnitsToStringMap))
            );
        if (*BACnetUnitsPtr == UINT32(-1))
        {
            return ERROR_NOT_SUPPORTED;
        }

        return ERROR_SUCCESS;
    }


	_Use_decl_annotations_
	DWORD
	FromString(const wchar_t* BACnetPolarityWsz, BACNET_POLARITY* BACnetPolarityPtr)
	{
		DSB_ASSERT(BACnetPolarityPtr != nullptr);

		*BACnetPolarityPtr = BACNET_POLARITY(
			bacnetObjectFromString(BACnetPolarityWsz, bacnetPolarityToStringMap, ARRAYSIZE(bacnetPolarityToStringMap))
			);
		if (*BACnetPolarityPtr == UINT32(-1))
		{
			return ERROR_NOT_SUPPORTED;
		}

		return ERROR_SUCCESS;
	}


    DWORD
    GetWin32Code(BACNET_ERROR_CODE BACnetErrorCode)
    {
        DWORD status = ERROR_GEN_FAILURE;

        switch (BACnetErrorCode)
        {
        case ERROR_CODE_DEVICE_BUSY:
            status = ERROR_BUSY;
            break;

        case ERROR_CODE_CONFIGURATION_IN_PROGRESS:
            status = ERROR_OPERATION_IN_PROGRESS;
            break;

        case ERROR_CODE_UNKNOWN_OBJECT:
        case ERROR_CODE_UNKNOWN_PROPERTY:
            status = ERROR_NOT_FOUND;
            break;

        case ERROR_CODE_INVALID_DATA_TYPE:
            __fallthrough;
        case ERROR_CODE_INVALID_TAG:
            __fallthrough;
        case ERROR_CODE_PARAMETER_OUT_OF_RANGE:
            __fallthrough;
        case ERROR_CODE_VALUE_OUT_OF_RANGE:
            status = ERROR_INVALID_DATA;
            break;

        case ERROR_CODE_ACCESS_DENIED:
            __fallthrough;
        case ERROR_CODE_WRITE_ACCESS_DENIED:
            status = ERROR_ACCESS_DENIED;
            break;

        case ERROR_CODE_TIMEOUT:
            status = ERROR_TIMEOUT;
            break;

        case ERROR_CODE_NOT_CONFIGURED:
            status = ERROR_NOT_READY;
            break;
        }

        return status;
    }

} // namespace AdapterLib
