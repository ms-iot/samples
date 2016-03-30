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

/**
 * This file contains the exported symbol.
 */

#include "DiscomfortIndexSensor.h"

#include <iostream>
#include "SysTimer.h"

#ifdef __ANDROID__
#include "OCAndroid.h"
#endif

using namespace DiscomfortIndexSensorName;

DiscomfortIndexSensor::DiscomfortIndexSensor()
{
    m_humidity = "";
    m_temperature = "";
    m_discomfortIndex = "";
}

DiscomfortIndexSensor::~DiscomfortIndexSensor()
{

}

int DiscomfortIndexSensor::executeDISensorLogic(std::map<std::string, std::string> *pInputData,
        std::string *pOutput)
{
    std::cout << "[DiscomfortIndexSensor] DiscomfortIndexSensor::" << __func__ << " is called."
              << std::endl;

    DIResult result;

    m_temperature = pInputData->at("temperature");
    m_humidity = pInputData->at("humidity");

    if ((result = makeDiscomfortIndex()) != SUCCESS)
    {
        std::cout << "Error : makeDiscomfortIndex() result = " << result << std::endl;
        return -1;
    }

    (*pOutput) = m_discomfortIndex;

    return 0;
}

/**
 * Calculation of DiscomfortIndex with TEMP&HUMI.
 */
DIResult DiscomfortIndexSensor::makeDiscomfortIndex()
{
    int DILevel = (int) ERROR;
    double dDI = 0.0;

    int t = std::stoi(m_temperature);
    int h = std::stoi(m_humidity);
    double F = (9.0 * (double) t) / 5.0 + 32.0;

    // calculation of discomfortIndex
    dDI = F - (F - 58.0) * (double)((100 - h) * 55) / 10000.0;

    std::cout << "Discomfort level : " << dDI << ", Temperature :" << t << ", Humidity :" << h <<
              std::endl;

    std::cout << "[result] Discomfort Index : " << m_discomfortIndex << std::endl;
    if (dDI >= 80.0)
    {
        DILevel = (int)ALL_DISCOMPORT;
        std::cout << "DI : " << DILevel << " : All person discomfort. : " << dDI
                  << std::endl;
    }
    else if (dDI >= 75.0)
    {
        DILevel = (int)HALF_DISCOMPORT;
        std::cout << "DI : " << DILevel << " : Half of person discomfort. : " << dDI
                  << std::endl;
    }
    else if (dDI >= 68.0)
    {
        DILevel = (int)LITTLE_DISCOMPORT;
        std::cout << "DI : " << DILevel << " : A little person discomfort. : " << dDI
                  << std::endl;
    }
    else
    {
        DILevel = (int)ALL_COMPORT;
        std::cout << "DI : " << DILevel << " : All person comfort. : " << dDI
                  << std::endl;
    }

    std::cout << std::endl;

    m_discomfortIndex = std::to_string(DILevel);

    return SUCCESS;
}