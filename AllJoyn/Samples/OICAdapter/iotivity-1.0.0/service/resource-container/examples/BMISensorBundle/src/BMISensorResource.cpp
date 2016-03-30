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

#include "BMISensorResource.h"


BMISensorResource::BMISensorResource()
{
    m_pBMISensor = new BMISensor();
}

BMISensorResource::~BMISensorResource()
{
    delete m_pBMISensor;
}

void BMISensorResource::handleSetAttributesRequest(
    RCSResourceAttributes &value)
{
    BundleResource::setAttributes(value);
}

RCSResourceAttributes &BMISensorResource::handleGetAttributesRequest()
{
    return BundleResource::getAttributes();
}

void BMISensorResource::executeLogic()
{
    std::string strBMIResult;

    if (m_pBMISensor->executeBMISensorLogic(&m_mapInputData, &strBMIResult) != -1)
        setAttribute("BMIresult", RCSResourceAttributes::Value(strBMIResult.c_str()));
}

void BMISensorResource::onUpdatedInputResource(const std::string attributeName,
        std::vector<RCSResourceAttributes::Value> values)
{
    // remove all existing data
    m_mapInputData.clear();

    if (!attributeName.compare("weight"))
        m_mapInputData.insert(std::make_pair("weight", values.back().toString()));

    if (!attributeName.compare("height"))
        m_mapInputData.insert(std::make_pair("height", values.back().toString()));

    executeLogic();
}