//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

///
///This sample demonstrates platform and device discovery feature
///The server sets the platform and device related info. which can
///be later retrieved by a client.
///

#include <mutex>
#include <condition_variable>

#include "OCPlatform.h"
#include "OCApi.h"

using namespace OC;

//Set of strings for each of deviceInfo fields
std::string dateOfManufacture = "myDateOfManufacture";
std::string firmwareVersion = "my.Firmware.Version";
std::string manufacturerName = "myName";
std::string operatingSystemVersion = "myOS";
std::string hardwareVersion = "myHardwareVersion";
std::string platformID = "myPlatformID";
std::string manufacturerUrl = "www.myurl.com";
std::string modelNumber = "myModelNumber";
std::string platformVersion = "platformVersion";
std::string supportUrl = "www.mysupporturl.com";
std::string systemTime = "mySystemTime";

//Set of strings for each of platform info fields
std::string deviceName = "Bill's Battlestar";

//OCPlatformInfo Contains all the platform info to be stored
OCPlatformInfo platformInfo;

//OCDeviceInfo Contains all the device info to be stored
OCDeviceInfo deviceInfo;

void DeletePlatformInfo()
{
    delete[] platformInfo.platformID;
    delete[] platformInfo.manufacturerName;
    delete[] platformInfo.manufacturerUrl;
    delete[] platformInfo.modelNumber;
    delete[] platformInfo.dateOfManufacture;
    delete[] platformInfo.platformVersion;
    delete[] platformInfo.operatingSystemVersion;
    delete[] platformInfo.hardwareVersion;
    delete[] platformInfo.firmwareVersion;
    delete[] platformInfo.supportUrl;
    delete[] platformInfo.systemTime;
}


void DeleteDeviceInfo()
{
    delete[] deviceInfo.deviceName;
}

void DuplicateString(char ** targetString, std::string sourceString)
{
    *targetString = new char[sourceString.length() + 1];
    strncpy(*targetString, sourceString.c_str(), (sourceString.length() + 1));
}

OCStackResult SetPlatformInfo(std::string platformID, std::string manufacturerName,
    std::string manufacturerUrl, std::string modelNumber, std::string dateOfManufacture,
    std::string platformVersion, std::string operatingSystemVersion, std::string hardwareVersion,
    std::string firmwareVersion, std::string supportUrl, std::string systemTime)
{
    DuplicateString(&platformInfo.platformID, platformID);
    DuplicateString(&platformInfo.manufacturerName, manufacturerName);
    DuplicateString(&platformInfo.manufacturerUrl, manufacturerUrl);
    DuplicateString(&platformInfo.modelNumber, modelNumber);
    DuplicateString(&platformInfo.dateOfManufacture, dateOfManufacture);
    DuplicateString(&platformInfo.platformVersion, platformVersion);
    DuplicateString(&platformInfo.operatingSystemVersion, operatingSystemVersion);
    DuplicateString(&platformInfo.hardwareVersion, hardwareVersion);
    DuplicateString(&platformInfo.firmwareVersion, firmwareVersion);
    DuplicateString(&platformInfo.supportUrl, supportUrl);
    DuplicateString(&platformInfo.systemTime, systemTime);
    return OC_STACK_OK;
}


OCStackResult SetDeviceInfo(std::string deviceName)
{
    DuplicateString(&deviceInfo.deviceName, deviceName);
    return OC_STACK_OK;
}


int main()
{
    // Create PlatformConfig object
    PlatformConfig cfg {
        OC::ServiceType::InProc,
        OC::ModeType::Server,
        "0.0.0.0", // By setting to "0.0.0.0", it binds to all available interfaces
        0,         // Uses randomly available port
        OC::QualityOfService::LowQos
    };

    OCPlatform::Configure(cfg);

    std::cout<<"Starting server & setting platform info\n";

    OCStackResult result = SetPlatformInfo(platformID, manufacturerName, manufacturerUrl,
            modelNumber, dateOfManufacture, platformVersion,  operatingSystemVersion,
            hardwareVersion, firmwareVersion,  supportUrl, systemTime);

    result = OCPlatform::registerPlatformInfo(platformInfo);

    if(result != OC_STACK_OK)
    {
        std::cout << "Platform Registration failed\n";
        return -1;
    }


    result = SetDeviceInfo(deviceName);

    result = OCPlatform::registerDeviceInfo(deviceInfo);

    if(result != OC_STACK_OK)
    {
        std::cout << "Device Registration failed\n";
        return -1;
    }

    DeletePlatformInfo();
    DeleteDeviceInfo();

    // A condition variable will free the mutex it is given, then do a non-
    // intensive block until 'notify' is called on it.  In this case, since we
    // don't ever call cv.notify, this should be a non-processor intensive version
    // of while(true);
    std::mutex blocker;
    std::condition_variable cv;
    std::unique_lock<std::mutex> lock(blocker);
    cv.wait(lock, []{return false;});

    // No explicit call to stop the platform.
    // When OCPlatform::destructor is invoked, internally we do platform cleanup

    return 0;

}




