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
#ifndef SIMULATOR_PLATFORM_INFO_H_
#define SIMULATOR_PLATFORM_INFO_H_

#include <iostream>

/**
 * @class   PlatformInfo
 *
 * @brief   This class contains remote device platform information and provide APIs access it.
 */
class PlatformInfo
{
    public:
        /**
         * This method is for getting platfom id.
         *
         * @return Platform id.
         */
        std::string getPlatformID() const;

        /**
         * This method is for getting platform version.
         *
         * @return Platform version.
         */
        std::string getPlatformVersion() const;

        /**
         * This method is for getting manufacturer name.
         *
         * @return manufacturer name.
         */
        std::string getManufacturerName() const;

        /**
         * This method is for getting manufacturer URL.
         *
         * @return manufacturer URL.
         */
        std::string getManufacturerUrl() const;

        /**
         * This method is for getting model number.
         *
         * @return Model number.
         */
        std::string getModelNumber() const;

        /**
         * This method is for getting date of manufacture.
         *
         * @return Date of manufacture.
         */
        std::string getDateOfManfacture() const;

        /**
         * This method is for getting operating system version.
         *
         * @return Operating system version.
         */
        std::string getOSVersion() const;

        /**
         * This method is for getting hardware version.
         *
         * @return Hardware version.
         */
        std::string getHardwareVersion() const;

        /**
         * This method is for getting firmware version.
         *
         * @return Firmware version.
         */
        std::string getFirmwareVersion() const;

        /**
         * This method is for getting support link URL.
         *
         * @return URL of support link.
         */
        std::string getSupportUrl() const;

        /**
         * This method is for getting system time.
         *
         * @return System time.
         */
        std::string getSystemTime() const;

        void setPlatformID(const std::string &platformId);
        void setPlatformVersion(const std::string &platformVersion);
        void setManufacturerName(const std::string &manufacturerName);
        void setManufacturerUrl(const std::string &manufacturerUrl);
        void setModelNumber(const std::string &modelNumber);
        void setDateOfManfacture(const std::string &dateOfManufacture);
        void setOSVersion(const std::string &osVersion);
        void setHardwareVersion(const std::string &hwVersion);
        void setFirmwareVersion(const std::string &firmwareVersion);
        void setSupportUrl(const std::string &supportUrl);
        void setSystemTime(const std::string &systemTime);

    private:
        std::string m_platformID;
        std::string m_manufacturerName;
        std::string m_manufacturerUrl;
        std::string m_modelNumber;
        std::string m_dateOfManufacture;
        std::string m_platformVersion;
        std::string m_operationSystemVersion;
        std::string m_hardwareVersion;
        std::string m_firmwareVersion;
        std::string m_supportUrl;
        std::string m_systemTime;
};

#endif
