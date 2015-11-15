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
#include "LSFEnums.h"

namespace BridgeRT
{

    public ref struct State sealed
    {
    public:
        property bool IsOn;
        property uint32 Brightness;
        property uint32 Hue;
        property uint32 Saturation;
        property uint32 ColorTemp;
    };

    // *********************************************************************************************************************************************************
    //
    // The interface between the Adapter Devices and the Lighting Service Framework
    //
    // *********************************************************************************************************************************************************
    public interface class ILSFHandler
    {
    public:

        // ********************************************************************LampService Interface************************************************************
        //
        //  The "Version" property of the LampService Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampService_Version
        {
            uint32 get();
        }

        // ********************************************************************LampService Interface************************************************************
        //
        //  The "LampServiceVersion" property of the LampService Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampService_LampServiceVersion
        {
            uint32 get();
        }

        // *********************************************************************LampService Interface***********************************************************
        //
        //  The "LampFaults" property of the LampService Interface.
        //
        // *****************************************************************************************************************************************************
        property Platform::Array<uint32>^ LampService_LampFaults
        {
            Platform::Array<uint32>^ get();
        }

        // *********************************************************************LampService Interface***********************************************************
        //
        //  The "ClearLampFault" method of the LampService Interface.
        //
        //  Input Parameters:
        //      - InLampFaultCode : The fault to be cleared
        //
        //  Output Parameters:
        //      - LampResponseCode  : The result code of the operation
        //      - OutLampFaultCode  : The fault code that was cleared
        //
        // *****************************************************************************************************************************************************
        uint32 ClearLampFault(
            _In_ uint32 InLampFaultCode,
            _Out_ uint32 *LampResponseCode,
            _Out_ uint32 *OutLampFaultCode
            );





        // ********************************************************************LampParameters Interface*********************************************************
        //
        //  The "Version" property of the LampParameters Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampParameters_Version
        {
            uint32 get();
        }

        // ********************************************************************LampParameters Interface*********************************************************
        //
        //  The "Energy_Usage_Milliwatts" property of the LampParameters Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampParameters_EnergyUsageMilliwatts
        {
            uint32 get();
        }

        // ********************************************************************LampParameters Interface*********************************************************
        //
        //  The "Brightness_Lumens" property of the LampParameters Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampParameters_BrightnessLumens
        {
            uint32 get();
        }





        // ********************************************************************LampDetails Interface*********************************************************
        //
        //  The "Version" property of the LampDetails Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampDetails_Version
        {
            uint32 get();
        }

        // ********************************************************************LampDetails Interface*********************************************************
        //
        //  The "Make" property of the LampDetails Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampDetails_Make
        {
            uint32 get();
        }

        // ********************************************************************LampDetails Interface*********************************************************
        //
        //  The "Model" property of the LampDetails Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampDetails_Model
        {
            uint32 get();
        }

        // ********************************************************************LampDetails Interface*********************************************************
        //
        //  The "Type" property of the LampDetails Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampDetails_Type
        {
            uint32 get();
        }

        // ********************************************************************LampDetails Interface*********************************************************
        //
        //  The "LampType" property of the LampDetails Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampDetails_LampType
        {
            uint32 get();
        }

        // ********************************************************************LampDetails Interface*********************************************************
        //
        //  The "LampBaseType" property of the LampDetails Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampDetails_LampBaseType
        {
            uint32 get();
        }

        // ********************************************************************LampDetails Interface*********************************************************
        //
        //  The "LampBeamAngle" property of the LampDetails Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampDetails_LampBeamAngle
        {
            uint32 get();
        }

        // ********************************************************************LampDetails Interface*********************************************************
        //
        //  The "Dimmable" property of the LampDetails Interface.
        //
        // *****************************************************************************************************************************************************
        property bool LampDetails_Dimmable
        {
            bool get();
        }

        // ********************************************************************LampDetails Interface*********************************************************
        //
        //  The "Color" property of the LampDetails Interface.
        //
        // *****************************************************************************************************************************************************
        property bool LampDetails_Color
        {
            bool get();
        }

        // ********************************************************************LampDetails Interface*********************************************************
        //
        //  The "VariableColorTemp" property of the LampDetails Interface.
        //
        // *****************************************************************************************************************************************************
        property bool LampDetails_VariableColorTemp
        {
            bool get();
        }

        // ********************************************************************LampDetails Interface*********************************************************
        //
        //  The "HasEffects" property of the LampDetails Interface.
        //
        // *****************************************************************************************************************************************************
        property bool LampDetails_HasEffects
        {
            bool get();
        }

        // ********************************************************************LampDetails Interface*********************************************************
        //
        //  The "MinVoltage" property of the LampDetails Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampDetails_MinVoltage
        {
            uint32 get();
        }

        // ********************************************************************LampDetails Interface*********************************************************
        //
        //  The "MaxVoltage" property of the LampDetails Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampDetails_MaxVoltage
        {
            uint32 get();
        }

        // ********************************************************************LampDetails Interface*********************************************************
        //
        //  The "Wattage" property of the LampDetails Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampDetails_Wattage
        {
            uint32 get();
        }

        // ********************************************************************LampDetails Interface*********************************************************
        //
        //  The "IncandescentEquivalent" property of the LampDetails Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampDetails_IncandescentEquivalent
        {
            uint32 get();
        }

        // ********************************************************************LampDetails Interface*********************************************************
        //
        //  The "MaxLumens" property of the LampDetails Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampDetails_MaxLumens
        {
            uint32 get();
        }

        // ********************************************************************LampDetails Interface*********************************************************
        //
        //  The "MinTemperature" property of the LampDetails Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampDetails_MinTemperature
        {
            uint32 get();
        }

        // ********************************************************************LampDetails Interface*********************************************************
        //
        //  The "MaxTemperature" property of the LampDetails Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampDetails_MaxTemperature
        {
            uint32 get();
        }

        // ********************************************************************LampDetails Interface*********************************************************
        //
        //  The "ColorRenderingIndex" property of the LampDetails Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampDetails_ColorRenderingIndex
        {
            uint32 get();
        }

        // ********************************************************************LampDetails Interface*********************************************************
        //
        //  The "LampID" property of the LampDetails Interface.
        //
        // *****************************************************************************************************************************************************
        property Platform::String^ LampDetails_LampID
        {
            Platform::String^ get();
        }





        // ********************************************************************LampState Interface*********************************************************
        //
        //  The "Version" property of the LampState Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampState_Version
        {
            uint32 get();
        }

        // ********************************************************************LampState Interface*************************************************************
        //
        //  The "OnOff" property of the LampState Interface.
        //
        // *****************************************************************************************************************************************************
        property bool LampState_OnOff;

        // ********************************************************************LampState Interface**************************************************************
        //
        //  The "Brightness" property of the LampState Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampState_Brightness;

        // ********************************************************************LampState Interface**************************************************************
        //
        //  The "Hue" property of the LampState Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampState_Hue;

        // ********************************************************************LampState Interface**************************************************************
        //
        //  The "Saturation" property of the LampState Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampState_Saturation;

        // ********************************************************************LampState Interface**************************************************************
        //
        //  The "ColorTemp" property of the LampState Interface.
        //
        // *****************************************************************************************************************************************************
        property uint32 LampState_ColorTemp;


        // *********************************************************************LampState Interface*************************************************************
        //
        //  The "TransitionLampState" method of the LampState Interface.
        //
        //  Input Parameters:
        //      - Timestamp         : Timestamp (in ms) of when to start the transition
        //      - NewState          : New state of the lamp to transition to
        //      - TransitionPeriod  : Time period (in ms) to transition over to new state
        //
        //  Output Parameters:
        //      - LampResponseCode  : The result code of the operation
        //
        // *****************************************************************************************************************************************************
        uint32 TransitionLampState(
            _In_ uint64 Timestamp,
            _In_ State^ NewState,
            _In_ uint32 TransitionPeriod,
            _Out_ uint32 *LampResponseCode
            );

        // *********************************************************************LampState Interface*************************************************************
        //
        //  The "ApplyPulseEffect" method of the LampState Interface.
        //
        //  Input Parameters:
        //      - FromState : Current state of the lamp to transition from
        //      - NewState  : New state of the lamp to transition to
        //      - Period    : Time period (in ms) to transition over new state
        //      - Duration  : Time period (in ms) to remain in new state
        //      - NumPulses : Number of pulses
        //      - Timestamp : Timestamp (in ms) of when to start the pulses
        //
        //  Output Parameters:
        //      - LampResponseCode  : The result code of the operation
        //
        // *****************************************************************************************************************************************************
        uint32 LampState_ApplyPulseEffect(
            _In_ State^ FromState,
            _In_ State^ ToState,
            _In_ uint32 Period,
            _In_ uint32 Duration,
            _In_ uint32 NumPulses,
            _In_ uint64 Timestamp,
            _Out_ uint32 *LampResponseCode
            );

        // ********************************************************************LampState Interface**************************************************************
        //
        //  The Adapter Signal associated with the "LampStateChanged" signal of the LampState Interface.
        //
        //  Parameters:
        //      - LampID : String
        //
        // *****************************************************************************************************************************************************
        property IAdapterSignal^ LampState_LampStateChanged
        {
            IAdapterSignal^ get();
        }
    };
}