#pragma once

using namespace Platform;

namespace BridgeRT
{

    //
    // Constants class
    // Description:
    //  Adapter level signals and parameters names
    //
    public ref class Constants sealed
    {
    public:

        // Device Arrival Signal
        static property Platform::String^ DEVICE_ARRIVAL_SIGNAL
        {
            inline Platform::String^ get() { return device_arrival_signal; }
        }

        static property Platform::String^ DEVICE_ARRIVAL__DEVICE_HANDLE
        {
            inline Platform::String^ get() { return device_arrival__device_handle; }
        }


        // Device Removal Signal
        static property Platform::String^ DEVICE_REMOVAL_SIGNAL
        {
            inline Platform::String^ get() { return device_removal_signal; }
        }

        static property Platform::String^ DEVICE_REMOVAL__DEVICE_HANDLE
        {
            inline Platform::String^ get() { return device_removal__device_handle; }
        }


        // Change of Value Signal
        static property Platform::String^ CHANGE_OF_VALUE_SIGNAL
        {
            inline Platform::String^ get() { return change_of_value_signal; }
        }

        static property Platform::String^ COV__PROPERTY_HANDLE
        {
            inline Platform::String^ get() { return cov__property_handle; }
        }

        static property Platform::String^ COV__ATTRIBUTE_HANDLE
        {
            inline Platform::String^ get() { return cov__attribute_handle; }
        }


        // Lamp State Changed Signal
        static property Platform::String^ LAMP_STATE_CHANGED_SIGNAL_NAME
        {
            inline Platform::String^ get() { return lamp_state_changed_signal_name; }
        }

        static property Platform::String^ SIGNAL_PARAMETER__LAMP_ID__NAME
        {
            inline Platform::String^ get() { return lamp_id__parameter_name; }
        }

    private:
        static Platform::String^ device_arrival_signal;
        static Platform::String^ device_arrival__device_handle;
        static Platform::String^ device_removal_signal;
        static Platform::String^ device_removal__device_handle;
        static Platform::String^ change_of_value_signal;
        static Platform::String^ cov__property_handle;
        static Platform::String^ cov__attribute_handle;
        static Platform::String^ lamp_state_changed_signal_name;
        static Platform::String^ lamp_id__parameter_name;
    };

}