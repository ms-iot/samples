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

#include "simulator_utils.h"
#include "OCRepresentation.h"

std::string getPayloadString(const OC::OCRepresentation &rep)
{
    std::ostringstream data;
    OCRepPayload *payload = rep.getPayload();
    if (!payload)
    {
        return "Payload: No payload";
    }

    // URI
    data << "URI: " << payload->uri << std::endl;

    // Attributes
    data << "Attributes:" << std::endl;
    OCRepPayloadValue *values = payload->values;
    while (NULL != values)
    {
        data << values->name << ":" << rep.getValueToString(values->name) << std::endl;
        values = values->next;
    }

    return data.str();
}

std::string getRequestString(const std::map<std::string, std::string> &queryParams,
                             const OC::OCRepresentation &rep)
{
    std::ostringstream data;
    data << "qp: ";
    if (queryParams.size() > 0)
    {
        for (auto & qp : queryParams)
            data << qp.second << ",";
    }

    data << getPayloadString(rep);
    return data.str();
}

std::string getRequestString(const std::map<std::string, std::string> &queryParams)
{
    std::ostringstream data;
    data << "qp: ";
    if (queryParams.size() > 0)
    {
        for (auto & qp : queryParams)
            data << qp.second << ",";
    }

    data << "Payload:  No payload";
    return data.str();
}