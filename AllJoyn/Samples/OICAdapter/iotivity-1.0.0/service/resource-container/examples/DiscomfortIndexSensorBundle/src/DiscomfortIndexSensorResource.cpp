//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
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

#include "DiscomfortIndexSensorResource.h"

#include <string>
#include <sstream>


DiscomfortIndexSensorResource::DiscomfortIndexSensorResource()
{
    m_pDiscomfortIndexSensor = new DiscomfortIndexSensor();
}

DiscomfortIndexSensorResource::~DiscomfortIndexSensorResource()
{
    delete m_pDiscomfortIndexSensor;
}

void DiscomfortIndexSensorResource::handleSetAttributesRequest(
    RCSResourceAttributes &value)
{
    BundleResource::setAttributes(value);
}

RCSResourceAttributes &DiscomfortIndexSensorResource::handleGetAttributesRequest()
{
    return BundleResource::getAttributes();
}

void DiscomfortIndexSensorResource::executeLogic()
{
    std::string strDiscomfortIndex;

    m_pDiscomfortIndexSensor->executeDISensorLogic(&m_mapInputData, &strDiscomfortIndex);

    setAttribute("discomfortIndex", RCSResourceAttributes::Value(strDiscomfortIndex.c_str()));

    for (auto it : m_mapInputData)
    {
        setAttribute(it.first, RCSResourceAttributes::Value(it.second.c_str()));
    }
}

void DiscomfortIndexSensorResource::onUpdatedInputResource(const std::string attributeName,
        std::vector<RCSResourceAttributes::Value> values)
{
    double sum = 0;
    double dConvert;
    int inputCount = 0;
    std::string itString;

    for (auto it : values)
    {
        itString = it.toString();
        std::stringstream ss(itString); //turn the string into a stream
        ss >> dConvert; //convert
        sum += dConvert;
        ++inputCount;
    }

    double result = sum / inputCount;
    std::string indexCount;//string which will contain the indexCount
    std::stringstream convert; // stringstream used for the conversion
    convert << result;//add the value of Number to the characters in the stream
    indexCount = convert.str();//set indexCount to the content of the stream

    m_mapInputData[attributeName] = indexCount;

    // execute logic only if all the input data are ready
    if (m_mapInputData.find("temperature") != m_mapInputData.end()
        && m_mapInputData.find("humidity") != m_mapInputData.end())
    {
        executeLogic();
    }
}