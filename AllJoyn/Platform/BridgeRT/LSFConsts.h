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

namespace BridgeRT
{
    const char PROPERTY_VERSION_STR[]                   = "Version";


    // Lamp Service
    const char PROPERTY_LAMP_SERVICE_VERSION_STR[]      = "LampServiceVersion";
    const char PROPERTY_LAMP_FAULTS_STR[]               = "LampFaults";


    // Lamp Parameters
    const char PROPERTY_ENERGY_USAGE_MILLIWATTS_STR[]   = "Energy_Usage_Milliwatts";
    const char PROPERTY_BRIGHTNESS_LUMENS_STR[]         = "Brightness_Lumens";


    // Lamp Details
    const char PROPERTY_MAKE_STR[]                      = "Make";
    const char PROPERTY_MODEL_STR[]                     = "Model";
    const char PROPERTY_TYPE_STR[]                      = "Type";
    const char PROPERTY_LAMP_TYPE_STR[]                 = "LampType";
    const char PROPERTY_LAMP_BASE_TYPE_STR[]            = "LampBaseType";
    const char PROPERTY_LAMP_BEAM_ANGLE_STR[]           = "LampBeamAngle";
    const char PROPERTY_DIMMABLE_STR[]                  = "Dimmable";
    const char PROPERTY_COLOR_STR[]                     = "Color";
    const char PROPERTY_VARIABLE_COLOR_TEMP_STR[]       = "VariableColorTemp";
    const char PROPERTY_HAS_EFFECTS_STR[]               = "HasEffects";
    const char PROPERTY_MIN_VOLTAGE_STR[]               = "MinVoltage";
    const char PROPERTY_MAX_VOLTAGE_STR[]               = "MaxVoltage";
    const char PROPERTY_WATTAGE_STR[]                   = "Wattage";
    const char PROPERTY_INCANDESCENT_EQUIVALENT_STR[]   = "IncandescentEquivalent";
    const char PROPERTY_MAX_LUMENS_STR[]                = "MaxLumens";
    const char PROPERTY_MIN_TEMPERATURE_STR[]           = "MinTemperature";
    const char PROPERTY_MAX_TEMPERATURE_STR[]           = "MaxTemperature";
    const char PROPERTY_COLOR_RENDERING_INDEX_STR[]     = "ColorRenderingIndex";
    const char PROPERTY_LAMP_ID_STR[]                   = "LampID";


    // Lamp State
    const char PROPERTY_ON_OFF_STR[]                    = "OnOff";
    const char PROPERTY_HUE_STR[]                       = "Hue";
    const char PROPERTY_SATURATION_STR[]                = "Saturation";
    const char PROPERTY_COLOR_TEMP_STR[]                = "ColorTemp";
    const char PROPERTY_BRIGHTNESS_STR[]                = "Brightness";

    // Method ClearLampFault
    const char METHOD_CLEAR_LAMP_FAULT_STR[]                    = "ClearLampFault";
    const char METHOD_CLEAR_LAMP_FAULT_INPUT_SIGNATURE_STR[]    = "u";
    const char METHOD_CLEAR_LAMP_FAULT_OUTPUT_SIGNATURE_STR[]   = "uu";
    const char METHOD_CLEAR_LAMP_FAULT_ARG_NAMES_STR[]          = "LampFaultCode,LampResponseCode,LampFaultCode";

    const int METHOD_CLEAR_LAMP_FAULT_INPUT_COUNT   = 1;
    const int METHOD_CLEAR_LAMP_FAULT_OUTPUT_COUNT  = 2;

    // Method TransitionLampState
    const char METHOD_TRANSITION_LAMP_STATE_STR[]                   = "TransitionLampState";
    const char METHOD_TRANSITION_LAMP_STATE_INPUT_SIGNATURE_STR[]   = "ta{sv}u";
    const char METHOD_TRANSITION_LAMP_STATE_OUTPUT_SIGNATURE_STR[]  = "u";
    const char METHOD_TRANSITION_LAMP_STATE_ARG_NAMES_STR[]         = "Timestamp,NewState,TransitionPeriod,LampResponseCode";

    const int METHOD_TRANSITION_LAMP_STATE_INPUT_COUNT      = 3;
    const int METHOD_TRANSITION_LAMP_STATE_OUTPUT_COUNT     = 1;
   
    //Method ApplyPulseEffect
    const char METHOD_APPLY_PULSE_EFFECT_STR[]                      = "ApplyPulseEffect";
    const char METHOD_APPLY_PULSE_EFFECT_INPUT_SIGNATURE_STR[]      = "a{sv}a{sv}uuut";
    const char METHOD_APPLY_PULSE_EFFECT_OUTPUT_SIGNATURE_STR[]     = "u";
    const char METHOD_APPLY_PULSE_EFFECT_ARG_NAMES_STR[]            = "FromState,ToState,period,duration,numPulses,timestamp,LampResponseCode";

    const int METHOD_APPLY_PULSE_EFFECT_INPUT_COUNT     = 6;
    const int METHOD_APPLY_PULSE_EFFECT_OUTPUT_COUNT    = 1;

    //Signal LampStateChanged
    const char SIGNAL_LAMP_STATE_CHANGED_STR[]  = "LampStateChanged";
    const char SIGNAL_PARAMETER_LAMP_ID_STR[]   = "LampID";


    //Method Get (DBusProperties Interface)
    const char METHOD_GET_STR[]                     = "Get";
    const char METHOD_GET_INPUT_SIGNATURE_STR[]     = "ss";
    const char METHOD_GET_OUTPUT_SIGNATURE_STR[]    = "v";

    //Method Set (DBusProperties Interface)
    const char METHOD_SET_STR[]                 = "Set";
    const char METHOD_SET_INPUT_SIGNATURE_STR[] = "ssv";

    //Method GetAll (DBusProperties Interface)
    const char METHOD_GET_ALL_STR[]                     = "GetAll";
    const char METHOD_GET_ALL_INPUT_SIGNATURE_STR[]     = "s";
    const char METHOD_GET_ALL_OUTPUT_SIGNATURE_STR[]    = "a{sv}";


    //Bus Object Path
    const char LSF_BUS_OBJECT_PATH[] = "/org/allseen/LSF/Lamp";

    // Interface names
    const char LAMP_DBUS_PROPERTIES_INTERFACE_NAME[]    = "org.freedesktop.DBus.Properties";
    const char LAMP_SERVICE_INTERFACE_NAME[]            = "org.allseen.LSF.LampService";
    const char LAMP_PARAMETERS_INTERFACE_NAME[]         = "org.allseen.LSF.LampParameters";
    const char LAMP_DETAILS_INTERFACE_NAME[]            = "org.allseen.LSF.LampDetails";
    const char LAMP_STATE_INTERFACE_NAME[]              = "org.allseen.LSF.LampState";

    // Lamp State Element Indices
    const int LAMP_STATE_ELEMENT_COUNT      = 5;  //OnOff, Brightness, Hue, Saturation, ColorTemp
    const int LAMP_STATE_ON_OFF_INDEX       = 0;
    const int LAMP_STATE_BRIGHTNESS_INDEX   = 1;
    const int LAMP_STATE_HUE_INDEX          = 2;
    const int LAMP_STATE_SATURATION_INDEX   = 3;
    const int LAMP_STATE_COLORTEMP_INDEX    = 4;

    // Signatures
    const char ARG_STRING_STR[]         = "s";
    const char ARG_BOOLEAN_STR[]        = "b";
    const char ARG_UINT32_STR[]         = "u";
    const char ARG_UINT32_ARRY_STR[]    = "au";
}