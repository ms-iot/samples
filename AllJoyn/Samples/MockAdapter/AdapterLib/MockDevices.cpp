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

#include "pch.h"
#include "MockAdapter.h"
#include "MockAdapterDevice.h"
#include "MockDevices.h"
#include <Windows.h>

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;

using namespace BridgeRT;

namespace AdapterLib
{
    //
    // Mock devices information
    //
    MOCK_DEVICE_DESCRIPTOR MockDevices[] =
    {
        // Device #1 - A light bulb
        {
            1,                        // Device ID
            L"Mock BACnet Switch",    // Device name
            L"Microsoft",             // Vendor
            L"2 X Switch",            // Model
            L"001-001-001",           // Serial
            L"1.1.1.1",               // Version
            L"2 Gang Switch Pack",    // Description

            // Properties
            {
                // Property  #1
                {
                    1,                          // Property ID
                    OBJECT_BINARY_OUTPUT,       // Property type
                    L"Switch 1",                // Name
                    L"BinaryOutput",            //Interface Hint
                    AdapterPropertyAccessRW,        // Access
                    UNITS_NO_UNITS,             // Units

                    // Current value
                    {
                        PropertyType::Boolean,  // ValueType
                        L"Room 1 Back Light",   // Name
                        {                       // Initial value
                            {0},    // SimpleType
                            {},     // Array of bytes
                            {L""}   // String
                        }
                    }
                },

                // Property  #2
                {
                    2,                          // Property ID
                    OBJECT_BINARY_OUTPUT,       // Property type
                    L"Switch 2",                // Name
                    L"BinaryOutput",            //Interface Hint
                    AdapterPropertyAccessRW,        // Access
                    UNITS_NO_UNITS,             // Units

                    // Current value
                    {
                        PropertyType::Boolean,  // ValueType
                        L"Room 1 Main Light",   // Name
                        {                       // Initial value
                            {0},    // SimpleType
                            {},     // Array of bytes
                            {L""}   // String
                        },
                    }
                },

                { LAST_DESCRIPTOR_ID } // Last property

            }, // Properties[]
            
            L"Light.bmp",
        },

        // Device #2 - A dimmable light bulb
        {
            2,                                  // Device ID
            L"Mock BACnet Dimmable Switch",     // Device name
            L"Microsoft",                       // Vendor
            L"Dim Control 725",                 // Model
            L"001-002-001",                     // Serial
            L"1.1.2.1",                         // Version
            L"Room2, Dimmable Light Switch",    // Description

            // Properties
            {
                // Property  #1
                {
                    1,                          // Property ID
                    OBJECT_BINARY_OUTPUT,       // Property type
                    L"Switch",                  // Name
                    L"BinaryOutput",            //Interface Hint
                    AdapterPropertyAccessRW,        // Access
                    UNITS_NO_UNITS,             // Units

                    // Current value
                    {
                        PropertyType::Boolean,  // ValueType
                        L"Living Room Light",   // Name
                        {                       // Initial value
                            {0},    // SimpleType
                            {},     // Array of bytes
                            {L""}   // String
                        },
                    }
                },

                // Property  #2
                {
                    2,                          // Property ID
                    OBJECT_ANALOG_OUTPUT,       // Property type
                    L"Dim Control",             // Name
                    L"AnalogOutput",            //Interface Hint
                    AdapterPropertyAccessRW,        // Access
                    UNITS_PERCENT,              // Units

                    // Current Value
                    {
                        PropertyType::UInt32,   // ValueType
                        L"Living Room Dimmer",  // Name
                        {                       // Initial value
                            {50},   // SimpleType
                            {},     // Array of bytes
                            {L""}   // WCHAR string
                        },
                    }
                },

                { LAST_DESCRIPTOR_ID } // Last property

            }, // Properties[]
        
            L"DimmableLight.jpg"
        },

        // Device #3 - A temperature sensor
        {
            3,                                  // Device ID
            L"Mock BACnet Temperature Sensor",  // Device name
            L"Microsoft",                       // Vendor
            L"Temperature Sensor 155",          // Model
            L"001-003-001",                     // Serial
            L"1.1.3.1",                         // Version
            L"Temperature Sensor",              // Description

            // Properties
            {
                // Property  #1
                {
                    1,                          // Property ID
                    OBJECT_ANALOG_INPUT,        // Property type
                    L"Temperature",             // Name
                    L"AnalogInput",             //Interface Hint
                    AdapterPropertyAccessRO,        // Access
                    UNITS_DEGREES_FAHRENHEIT,   // Units

                    // Current Value
                    {
                        PropertyType::Double,       // ValueType
                        L"Living Room Temperature", // Name
                        {                           // Initial value
                            {62.5}, // SimpleType
                            {},     // Array of bytes
                            {L""}   // WCHAR string
                        },
                    }
                },

                { LAST_DESCRIPTOR_ID } // Last endpoint
            },
            L"Thermometer.png"
        },
    };
    ULONG MockDevicesCount = ARRAYSIZE(MockDevices);


    String^ ToString(ADAPTER_MOCK_UNITS PropertyUnits)
    {
        String^ unitsStr = ref new String(L"Unsupported");

        switch (PropertyUnits)
        {
        case UNITS_NO_UNITS:
            unitsStr = L"UNITS_NO_UNITS";
            break;
        case UNITS_PERCENT:
            unitsStr = L"UNITS_PERCENT";
            break;
        case UNITS_DEGREES_FAHRENHEIT:
            unitsStr = L"UNITS_DEGREES_FAHRENHEIT";
            break;
        }

        return unitsStr;
    }


    String^ ToString(ADAPTER_MOCK_PROPERTY_ACCESS PropertyAccess)
    {
        String^ accessStr = ref new String(L"Unsupported");

        switch (PropertyAccess)
        {
        case AdapterPropertyAccessInvalid:
            accessStr = L"Invalid Access Code";
            break;
        case AdapterPropertyAccessRO:
            accessStr = L"RO";
            break;
        case AdapterPropertyAccessRW:
            accessStr = L"RW";
            break;
        }

        return accessStr;
    }


    String^ ToString(ADAPTER_MOCK_PROPERTY_TYPE PropertyType)
    {
        String^ typeStr = ref new String(L"Unsupported");

        switch (PropertyType)
        {
        case OBJECT_ANALOG_INPUT:
            typeStr = L"OBJECT_ANALOG_INPUT";
            break;
        case OBJECT_ANALOG_OUTPUT:
            typeStr = L"OBJECT_ANALOG_OUTPUT";
            break;
        case OBJECT_ANALOG_VALUE:
            typeStr = L"OBJECT_ANALOG_VALUE";
            break;
        case OBJECT_BINARY_INPUT:
            typeStr = L"OBJECT_BINARY_INPUT";
            break;
        case OBJECT_BINARY_OUTPUT:
            typeStr = L"OBJECT_BINARY_OUTPUT";
            break;
        case OBJECT_BINARY_VALUE:
            typeStr = L"OBJECT_BINARY_VALUE";
            break;
        }

        return typeStr;
    }


    String^ ToString(ADAPTER_MOCK_PROPERTY_ATTRIBUTE_ID PropertyAttributeId)
    {
        String^ attrIdStr = ref new String(L"Unsupported");

        switch (PropertyAttributeId)
        {
        case PROP_NAME:
            attrIdStr = L"Object_Name";
            break;
        case PROP_TYPE:
            attrIdStr = L"Object_Type";
            break;
        case PROP_ID:
            attrIdStr = L"Object_Identifier";
            break;
        case PROP_ACCESS:
            attrIdStr = L"Access";
            break;
        case PROP_UNITS:
            attrIdStr = L"Units";
            break;
        case PROP_PRESENT_VALUE:
            attrIdStr = L"Present_Value";
            break;
        }

        return attrIdStr;
    }

} // AdapterLib
