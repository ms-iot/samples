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

#ifndef DISCOMFORTINDEXSENSOR_H_
#define DISCOMFORTINDEXSENSOR_H_

#include <map>
#include <string>

namespace DiscomfortIndexSensorName
{
    typedef enum
    {
        SUCCESS = 0, ERROR, ALL_DISCOMPORT, HALF_DISCOMPORT, LITTLE_DISCOMPORT, ALL_COMPORT
    } DIResult;

    class DiscomfortIndexSensor
    {
        public:
            DiscomfortIndexSensor();
            ~DiscomfortIndexSensor();

            int executeDISensorLogic(std::map<std::string, std::string> *pInputData,
                                     std::string *pOutput);
            DIResult makeDiscomfortIndex();

        private:
            std::string m_humidity;
            std::string m_temperature;
            std::string m_discomfortIndex;
    };
};

#endif /* DISCOMFORTINDEXSENSOR_H_ */
