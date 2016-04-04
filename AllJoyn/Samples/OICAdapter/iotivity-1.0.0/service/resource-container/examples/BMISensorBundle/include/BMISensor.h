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

#ifndef BMISENSOR_H_
#define BMISENSOR_H_

#include <string>
#include <map>
#include <time.h>

/**
 * Who BMI
 * weight / height *height
 *
 * < 18.5  : underweight
 * 18.5 <= < 25 : Normal Range
 * <= 25  < 30 : Overweight
 * <= 30 : Obese
 */

#define UNKOWNBMI   -1
#define OUTOFDATEBMI   0
#define UNDERWEIGHT     1
#define NORMALRANGE     2
#define OVERWEIGHT      3
#define OBESE           4

#define UNDERWEIGHT_VAL     18.5
#define NORMALRANGE_VAL     25.9
#define OVERWEIGHT_VAL      30

#define DIFFTIME      5     // valid time difference. (seconds)


namespace BMISensorName
{
    typedef enum
    {
        SUCCESS = 0, ERROR
    } BMIResult;

    class BMISensor
    {
        public:
            BMISensor();
            ~BMISensor();

            int executeBMISensorLogic(std::map<std::string, std::string> *pInputData,
                                      std::string *pOutput);
            BMIResult makeBMI(void);

        private:
            std::string m_weight;
            std::string m_height;
            std::string m_BMIResult;

            time_t  m_timepstampW;
            time_t  m_timepstampH;
    };
};

#endif /* BMISENSOR_H_ */
