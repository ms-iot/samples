// Copyright (c) Microsoft. All rights reserved.
#include "pch.h"
#include "MACFinder.h"
#include <winsock2.h>
#include <iphlpapi.h>
#include <ipifcons.h>

using namespace IoTOnboardingUtils;


#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

Platform::String^ MACFinder::GetWiFiAdapterMAC()
{
#ifndef IF_TYPE_IEEE80211
    const IFTYPE IF_TYPE_IEEE80211 = 71;
#endif
    const size_t MAX_PHYS_ADDRESS_LENGTH = 32;
    const size_t WORKING_BUFFER_SIZE = 15000;
    const ULONG  MAX_ATTEMPTS = 3;

    DWORD dwRetVal = 0;
    ULONG outBufLen = 0;

    ULONG family = AF_UNSPEC;
    ULONG attempt = 0;
    PIP_ADAPTER_ADDRESSES pAddresses = nullptr;
    PIP_ADAPTER_ADDRESSES pCurrAddresses = nullptr;
    Platform::String^ outputString = nullptr;

    try
    {
        // Get the Adapter Address Information for all adapters... 
        // Iterate (up to 3 times) to get a buffer large enough 
        // to hold the adapter information
        do
        {
            // Allocate a buffer
            pAddresses = static_cast<IP_ADAPTER_ADDRESSES*>(MALLOC(outBufLen));
            if (nullptr == pAddresses)
            {
                return nullptr;
            }

            // Try to get the adapter information
            dwRetVal = GetAdaptersAddresses(family, 0, nullptr, pAddresses, &outBufLen);
            
            // Break if we don't need to grow the buffer (result may not have been a success)
            if (dwRetVal != ERROR_BUFFER_OVERFLOW)
            {
                break;
            }

            // Iterate again with a larger buffer size
            FREE(pAddresses);
            pAddresses = nullptr;
            attempt++;
        } 
        while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (attempt < MAX_ATTEMPTS));


        //
        // If we successfully acquired the adapter information, then search it for the WiFi adapter's MAC address
        // 
        if (dwRetVal == NO_ERROR)
        {
            // Search the list of adapter addressses for the first WiFi adapter with a physical address value
            pCurrAddresses = pAddresses;
            while (pCurrAddresses != nullptr)
            {
                if ((pCurrAddresses->PhysicalAddressLength != 0) &&
                    (pCurrAddresses->IfType == IF_TYPE_IEEE80211))
                {
                    // A WiFi adapter was found
                    break;
                }
                pCurrAddresses = pCurrAddresses->Next;
            }

            // If we successfully found a wireless adapter, then Convert its MAC address to a platform string
            if (pCurrAddresses != nullptr)
            {
                const size_t BYTE_STRING_WIDTH = 2;
                wchar_t physAddressString[MAX_PHYS_ADDRESS_LENGTH] = { 0 };
                size_t outputIdx = 0;

                //Convert each byte of the physical address into a string
                for (ULONG i = 0; i < pCurrAddresses->PhysicalAddressLength; i++)
                {
                    auto remainingStringLength = MAX_PHYS_ADDRESS_LENGTH - outputIdx;                   
                    _snwprintf_s(&physAddressString[outputIdx], remainingStringLength, _TRUNCATE, L"%.2X", pCurrAddresses->PhysicalAddress[i]);
                    outputIdx += BYTE_STRING_WIDTH;
                }
                outputString = ref new Platform::String(physAddressString);
            }
        }

    }
    catch (...)
    {
    }

    if (pAddresses != nullptr)
    {
        FREE(pAddresses);
        pAddresses = nullptr;
    }

    return outputString;
}
