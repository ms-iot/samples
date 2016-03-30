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

#include "simulator_device_info.h"

DeviceInfo::DeviceInfo(const std::string &name, const std::string &id,
                       const std::string &specVersion, const std::string &dmv)
    : m_name(name), m_id(id), m_specVersion(specVersion), m_DMV(dmv) {}

std::string DeviceInfo::getName() const
{
    return m_name;
}

std::string DeviceInfo::getID() const
{
    return m_id;
}

std::string DeviceInfo::getSpecVersion() const
{
    return m_specVersion;
}

std::string DeviceInfo::getDataModelVersion() const
{
    return m_DMV;
}
