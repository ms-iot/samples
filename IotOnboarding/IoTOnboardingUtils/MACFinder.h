// Copyright (c) Microsoft. All rights reserved.
#pragma once

namespace IoTOnboardingUtils 
{
    public ref class MACFinder sealed
    {
    public:       
        static Platform::String^ GetWiFiAdapterMAC();
    };
};
