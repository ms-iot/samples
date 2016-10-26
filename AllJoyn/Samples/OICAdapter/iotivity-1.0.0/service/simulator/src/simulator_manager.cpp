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

#include "simulator_manager.h"
#include "resource_manager.h"
#include "simulator_client.h"
#include "simulator_utils.h"

SimulatorManager *SimulatorManager::getInstance()
{
    static SimulatorManager s_instance;
    return &s_instance;
}

SimulatorManager::SimulatorManager()
{
    OC::PlatformConfig conf
    {
        OC::ServiceType::InProc,
        OC::ModeType::Both,
        "0.0.0.0", // By setting to "0.0.0.0", it binds to all available interfaces
        0,         // Uses randomly available port
        OC::QualityOfService::LowQos
    };

    OC::OCPlatform::Configure(conf);
}

std::shared_ptr<SimulatorResourceServer> SimulatorManager::createResource(
    const std::string &configPath,
    SimulatorResourceServer::ResourceModelChangedCB callback)
{
    return ResourceManager::getInstance()->createResource(configPath, callback);
}

std::vector<std::shared_ptr<SimulatorResourceServer>> SimulatorManager::createResource(
            const std::string &configPath, unsigned short count,
            SimulatorResourceServer::ResourceModelChangedCB callback)
{
    return ResourceManager::getInstance()->createResource(configPath, count, callback);
}

std::vector<std::shared_ptr<SimulatorResourceServer>> SimulatorManager::getResources(
            const std::string &resourceType)
{
    return ResourceManager::getInstance()->getResources(resourceType);
}

void SimulatorManager::deleteResource(
    const std::shared_ptr<SimulatorResourceServer> &resource)
{
    ResourceManager::getInstance()->deleteResource(resource);
}

void SimulatorManager::deleteResource(const std::string &resourceType)
{
    ResourceManager::getInstance()->deleteResources(resourceType);
}

void SimulatorManager::findResource(ResourceFindCallback callback)
{
    SimulatorClient::getInstance()->findResources(callback);
}

void SimulatorManager::findResource(const std::string &resourceType,
                                     ResourceFindCallback callback)
{
    SimulatorClient::getInstance()->findResources(resourceType, callback);
}

void SimulatorManager::getDeviceInfo(DeviceInfoCallback callback)
{
    if (!callback)
        throw InvalidArgsException(SIMULATOR_INVALID_CALLBACK, "Invalid callback!");

    OC::FindDeviceCallback deviceCallback = [this, callback](const OC::OCRepresentation & rep)
    {
        std::string deviceName = rep.getValue<std::string>("n");
        std::string deviceID = rep.getValue<std::string>("di");
        std::string deviceSpecVersion = rep.getValue<std::string>("lcv");
        std::string deviceDMV = rep.getValue<std::string>("dmv");

        DeviceInfo deviceInfo(deviceName, deviceID, deviceSpecVersion, deviceDMV);
        callback(deviceInfo);
    };

    std::ostringstream uri;
    uri << OC_MULTICAST_PREFIX << OC_RSRVD_DEVICE_URI;

    typedef OCStackResult (*GetDeviceInfo)(const std::string &, const std::string &,
                                           OCConnectivityType, OC::FindDeviceCallback);

    invokeocplatform(static_cast<GetDeviceInfo>(OC::OCPlatform::getDeviceInfo), "",
                     uri.str(),
                     CT_DEFAULT,
                     deviceCallback);
}

void SimulatorManager::setDeviceInfo(const std::string &deviceName)
{
    if (deviceName.empty())
        throw InvalidArgsException(SIMULATOR_INVALID_PARAM, "Device name is empty!");


    typedef OCStackResult (*RegisterDeviceInfo)(const OCDeviceInfo);

    OCDeviceInfo ocDeviceInfo;
    ocDeviceInfo.deviceName = const_cast<char *>(deviceName.c_str());
    invokeocplatform(static_cast<RegisterDeviceInfo>(OC::OCPlatform::registerDeviceInfo),
                     ocDeviceInfo);
}

void SimulatorManager::getPlatformInfo(PlatformInfoCallback callback)
{
    if (!callback)
        throw InvalidArgsException(SIMULATOR_INVALID_CALLBACK, "Invalid callback!");

    OC::FindPlatformCallback platformCallback = [this, callback](const OC::OCRepresentation & rep)
    {
        PlatformInfo platformInfo;
        platformInfo.setPlatformID(rep.getValue<std::string>("pi"));
        platformInfo.setPlatformVersion(rep.getValue<std::string>("mnpv"));
        platformInfo.setManufacturerName(rep.getValue<std::string>("mnmn"));
        platformInfo.setManufacturerUrl(rep.getValue<std::string>("mnml"));
        platformInfo.setModelNumber(rep.getValue<std::string>("mnmo"));
        platformInfo.setDateOfManfacture(rep.getValue<std::string>("mndt"));
        platformInfo.setOSVersion(rep.getValue<std::string>("mnos"));
        platformInfo.setHardwareVersion(rep.getValue<std::string>("mnhw"));
        platformInfo.setFirmwareVersion(rep.getValue<std::string>("mnfv"));
        platformInfo.setSupportUrl(rep.getValue<std::string>("mnsl"));
        platformInfo.setSystemTime(rep.getValue<std::string>("st"));

        callback(platformInfo);
    };

    std::ostringstream uri;
    uri << OC_MULTICAST_PREFIX << OC_RSRVD_PLATFORM_URI;

    typedef OCStackResult (*GetPlatformInfo)(const std::string &, const std::string &,
            OCConnectivityType, OC::FindPlatformCallback);

    invokeocplatform(static_cast<GetPlatformInfo>(OC::OCPlatform::getPlatformInfo), "",
                     uri.str(),
                     CT_DEFAULT,
                     platformCallback);
}

void SimulatorManager::setPlatformInfo(PlatformInfo &platformInfo)
{
    OCPlatformInfo ocPlatformInfo;
    ocPlatformInfo.platformID = const_cast<char *>(platformInfo.getPlatformID().c_str());
    ocPlatformInfo.manufacturerName = const_cast<char *>(platformInfo.getManufacturerName().c_str());
    ocPlatformInfo.manufacturerUrl = const_cast<char *>(platformInfo.getManufacturerUrl().c_str());
    ocPlatformInfo.modelNumber = const_cast<char *>(platformInfo.getModelNumber().c_str());
    ocPlatformInfo.dateOfManufacture = const_cast<char *>(platformInfo.getDateOfManfacture().c_str());
    ocPlatformInfo.platformVersion = const_cast<char *>(platformInfo.getPlatformVersion().c_str());
    ocPlatformInfo.operatingSystemVersion = const_cast<char *>(platformInfo.getOSVersion().c_str());
    ocPlatformInfo.hardwareVersion = const_cast<char *>(platformInfo.getHardwareVersion().c_str());
    ocPlatformInfo.firmwareVersion = const_cast<char *>(platformInfo.getFirmwareVersion().c_str());
    ocPlatformInfo.supportUrl = const_cast<char *>(platformInfo.getSupportUrl().c_str());
    ocPlatformInfo.systemTime = const_cast<char *>(platformInfo.getSystemTime().c_str());

    typedef OCStackResult (*RegisterPlatformInfo)(const OCPlatformInfo);
    invokeocplatform(static_cast<RegisterPlatformInfo>(OC::OCPlatform::registerPlatformInfo),
                     ocPlatformInfo);
}

void SimulatorManager::setLogger(const std::shared_ptr<ILogger> &logger)
{
    simLogger().setCustomTarget(logger);
}

bool SimulatorManager::setConsoleLogger()
{
    return simLogger().setDefaultConsoleTarget();
}

bool SimulatorManager::setFileLogger(const std::string &path)
{
    return simLogger().setDefaultFileTarget(path);
}