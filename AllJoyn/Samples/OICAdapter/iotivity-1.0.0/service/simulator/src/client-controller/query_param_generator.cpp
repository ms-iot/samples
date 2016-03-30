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

#include "query_param_generator.h"

QPGenerator::QPGenerator(const std::map<std::string, std::vector<std::string>> &queryParams)
{
    for (auto entry : queryParams)
    {
        if (entry.second.size())
        {
            QPDetail detail;
            detail.key.assign(entry.first.c_str());
            detail.values = entry.second;
            detail.index = 0;
            m_qpDetails.push_back(detail);
        }
    }
}

bool QPGenerator::hasNext()
{
    if (m_qpDetails.size() &&
        (m_qpDetails[0].index < m_qpDetails[0].values.size()))
    {
        return true;
    }

    return false;
}

std::map<std::string, std::string> QPGenerator::next()
{
    std::map<std::string, std::string> queryParams;
    if (!hasNext())
        return queryParams;

    for (auto ele : m_qpDetails)
        queryParams[ele.key] = ele.values[ele.index];

    for (int index = m_qpDetails.size() - 1; index >= 0; index--)
    {
        m_qpDetails[index].index++;
        if (m_qpDetails[index].index >= m_qpDetails[index].values.size()) // Boundary check
        {
            if (index != 0)
            {
                m_qpDetails[index].index = 0;
                continue;
            }
        }
        break;
    }

    return queryParams;
}