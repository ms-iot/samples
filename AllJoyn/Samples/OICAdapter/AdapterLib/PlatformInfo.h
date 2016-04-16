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

#include "ocstack.h"
#include <string>

namespace AdapterLib
{
    ref class AdapterDevice;

    struct PlatformInfo
    {
    public:
        PlatformInfo(const OCPlatformInfo& info, const OCDevAddr& devAddr)
        {
            platformID              = info.platformID? info.platformID: "";
            manufacturerName        = info.manufacturerName? info.manufacturerName: "";
            manufacturerUrl         = info.manufacturerUrl? info.manufacturerUrl: "";
            modelNumber             = info.modelNumber? info.modelNumber: "";
            dateOfManufacture       = info.dateOfManufacture? info.dateOfManufacture: "";
            platformVersion         = info.platformVersion? info.platformVersion: "";
            operatingSystemVersion  = info.operatingSystemVersion? info.operatingSystemVersion: "";
            hardwareVersion         = info.hardwareVersion? info.hardwareVersion: "";
            firmwareVersion         = info.firmwareVersion? info.firmwareVersion: "";
            supportUrl              = info.supportUrl? info.supportUrl: "";
            systemTime              = info.systemTime? info.systemTime: "";

            memset(&addr, 0, sizeof(addr));
            memcpy(&addr, &devAddr, sizeof(addr));
        }

        PlatformInfo(const PlatformInfo& otherInfo)
        {
            platformID = otherInfo.platformID;
            manufacturerName = otherInfo.manufacturerName;
            manufacturerUrl = otherInfo.manufacturerUrl;
            modelNumber = otherInfo.modelNumber;
            dateOfManufacture = otherInfo.dateOfManufacture;
            platformVersion = otherInfo.platformVersion;
            operatingSystemVersion = otherInfo.operatingSystemVersion;
            hardwareVersion = otherInfo.hardwareVersion;
            firmwareVersion = otherInfo.firmwareVersion;
            supportUrl = otherInfo.supportUrl;
            systemTime = otherInfo.systemTime;

            memcpy(&addr, &otherInfo.addr, sizeof(addr));
        }

        bool operator==(const PlatformInfo& info)
        {
            bool bRet = false;
            if ((std::string(addr.addr) == info.addr.addr)
                && (addr.port == info.addr.port)
                && (platformID == info.platformID)
                && (manufacturerName == info.manufacturerName)
                && (manufacturerUrl == info.manufacturerUrl)
                && (modelNumber == info.modelNumber)
                && (dateOfManufacture == info.dateOfManufacture)
                && (platformVersion == info.platformVersion)
                && (operatingSystemVersion == info.operatingSystemVersion)
                && (hardwareVersion == info.hardwareVersion)
                && (firmwareVersion == info.firmwareVersion))
            {
                bRet = true;
            }
            return bRet;
        }

    public:
        std::string platformID;
        std::string manufacturerName;
        std::string manufacturerUrl;
        std::string modelNumber;
        std::string dateOfManufacture;
        std::string platformVersion;
        std::string operatingSystemVersion;
        std::string hardwareVersion;
        std::string firmwareVersion;
        std::string supportUrl;
        std::string systemTime;

        OCDevAddr addr;
    };

}   //namespace AdapterLib