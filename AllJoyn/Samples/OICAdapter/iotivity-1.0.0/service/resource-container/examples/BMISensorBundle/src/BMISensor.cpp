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
#include "BMISensor.h"

#include <iostream>
#include <stdlib.h>
#include "SysTimer.h"

#ifdef __ANDROID__
#include "OCAndroid.h"
#endif

using namespace BMISensorName;


BMISensor::BMISensor()
{
    m_weight = "";
    m_height = "";
    m_BMIResult = "";
    time(&m_timepstampW);
    time(&m_timepstampH);
}

BMISensor::~BMISensor()
{

}

int BMISensor::executeBMISensorLogic(std::map<std::string, std::string> *pInputData,
                                     std::string *pOutput)
{
    BMIResult result;

    if (pInputData->find("weight") != pInputData->end())
    {
        m_weight = pInputData->at("weight");
        time(&m_timepstampW);
    }

    if (pInputData->find("height") != pInputData->end())
    {
        m_height = pInputData->at("height");
        time(&m_timepstampH);
    }

    if ((result = makeBMI()) != SUCCESS)
    {
        return -1;
    }

    (*pOutput) = m_BMIResult;

    return 0;
}

/**
 * Calculation of BMI with Weight&Height
 */
BMIResult BMISensor::makeBMI(void)
{
    double BMIvalue, timediffsecond;
    double dWeight, dHeight;

    int BMIResult;

    if (!m_weight.empty() && !m_height.empty())
    {
        dWeight = std::stod(m_weight);
        dHeight = std::stod(m_height);

        timediffsecond = abs(difftime(m_timepstampW, m_timepstampH));

        // check if time difference between weight data and height data is valid
        if (timediffsecond > DIFFTIME)
        {
            BMIvalue = 0;
            BMIResult = UNKOWNBMI;
            std::cout << "[BMISensor] :   OUTOFDATEBMI: " << BMIResult << std::endl;
        }
        else if ((dWeight > 0) && (dHeight > 0))
        {
            // calculate BMI
            BMIvalue = dWeight / (dHeight * dHeight);

            std::cout << "[BMISensor] height : " << m_height << " weight : " << m_weight
                      << " BMIvalue : " << BMIvalue  << " timediff : " << timediffsecond
                      << std::endl;

            if (BMIvalue >= OVERWEIGHT_VAL)
            {
                BMIResult = (int)OBESE;
                std::cout << "[BMISensor] : BMIresult:" << BMIResult << " OBESE " << std::endl;
            }
            else if (BMIvalue >= NORMALRANGE_VAL )
            {
                BMIResult = (int)OVERWEIGHT;
                std::cout << "[BMISensor] : BMIresult:" << BMIResult << " OVERWEIGHT " << std::endl;

            }
            else if (BMIvalue >= UNDERWEIGHT_VAL )
            {
                BMIResult = (int)NORMALRANGE;
                std::cout << "[BMISensor] : BMIresult:" << BMIResult << " NORMALRANGE " << std::endl;

            }
            else
            {
                BMIResult = (int)UNDERWEIGHT;
                std::cout << "[BMISensor] : BMIresult:" << BMIResult << " UNDERWEIGHT " << std::endl;
            }
        }
        else
        {
            BMIvalue = -1;
            BMIResult = UNKOWNBMI;
            std::cout << "[BMISensor] :   UNKNOWNBMI: " << BMIResult << std::endl;
        }

        std::cout << std::endl;

        m_BMIResult = std::to_string(BMIResult);

        return SUCCESS;
    }

    return ERROR;
}