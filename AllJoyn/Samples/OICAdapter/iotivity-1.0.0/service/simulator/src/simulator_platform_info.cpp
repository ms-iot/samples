/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

#include "simulator_platform_info.h"

std::string PlatformInfo::getPlatformID() const
{
    return m_platformID;
}

std::string PlatformInfo::getPlatformVersion() const
{
    return m_platformVersion;
}

std::string PlatformInfo::getManufacturerName() const
{
    return m_manufacturerName;
}

std::string PlatformInfo::getManufacturerUrl() const
{
    return m_manufacturerUrl;
}

std::string PlatformInfo::getModelNumber() const
{
    return m_modelNumber;
}

std::string PlatformInfo::getDateOfManfacture() const
{
    return m_dateOfManufacture;
}

std::string PlatformInfo::getOSVersion() const
{
    return m_operationSystemVersion;
}

std::string PlatformInfo::getHardwareVersion() const
{
    return m_hardwareVersion;
}

std::string PlatformInfo::getFirmwareVersion() const
{
    return m_firmwareVersion;
}

std::string PlatformInfo::getSupportUrl() const
{
    return m_supportUrl;
}

std::string PlatformInfo::getSystemTime() const
{
    return m_systemTime;
}

void PlatformInfo::setPlatformID(const std::string &platformId)
{
    m_platformID = platformId;
}

void PlatformInfo::setPlatformVersion(const std::string &platformVersion)
{
    m_platformVersion = platformVersion;
}

void PlatformInfo::setManufacturerName(const std::string &manufacturerName)
{
    m_manufacturerName = manufacturerName;
}

void PlatformInfo::setManufacturerUrl(const std::string &manufacturerUrl)
{
    m_manufacturerUrl = manufacturerUrl;
}

void PlatformInfo::setModelNumber(const std::string &modelNumber)
{
    m_modelNumber = modelNumber;
}

void PlatformInfo::setDateOfManfacture(const std::string &dateOfManufacture)
{
    m_dateOfManufacture = dateOfManufacture;
}

void PlatformInfo::setOSVersion(const std::string &osVersion)
{
    m_operationSystemVersion = osVersion;
}

void PlatformInfo::setHardwareVersion(const std::string &hwVersion)
{
    m_hardwareVersion = hwVersion;
}

void PlatformInfo::setFirmwareVersion(const std::string &firmwareVersion)
{
    m_firmwareVersion = firmwareVersion;
}

void PlatformInfo::setSupportUrl(const std::string &supportUrl)
{
    m_supportUrl = supportUrl;
}

void PlatformInfo::setSystemTime(const std::string &systemTime)
{
    m_systemTime = systemTime;
}
