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

#ifndef SIMULATOR_DEVICE_INFO_H_
#define SIMULATOR_DEVICE_INFO_H_

#include <iostream>

/**
 * @class   DeviceInfo
 *
 * @brief   This class contains remote device information and provide APIs access it.
 */
class DeviceInfo
{
    public:
        DeviceInfo(const std::string &, const std::string &, const std::string &, const std::string &);

        /**
         * This method is for getting device name.
         *
         * @return Device name.
         */
        std::string getName() const;

        /**
         * This method is for getting device id.
         *
         * @return Device id.
         */
        std::string getID() const;

        /**
         * This method is for getting device specification version.
         *
         * @return Device id.
         */
        std::string getSpecVersion() const;

        /**
         * This method is for getting device data model version.
         *
         * @return Device data model version.
         */
        std::string getDataModelVersion() const;

    private:
        std::string m_name;
        std::string m_id;
        std::string m_specVersion;
        std::string m_DMV;
};

#endif
