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

#ifndef SOFTSENSORRESOURCE_H_
#define SOFTSENSORRESOURCE_H_

#include "BundleResource.h"

namespace OIC
{
    namespace Service
    {

        /**
        * @class    SoftSensorResource
        * @brief    This class represents bundle resource for Soft Sensor
        *               to be registered in the container and make resource server
        *
        */
        class SoftSensorResource: public BundleResource
        {
            public:
                /**
                * Constructor for SoftSensorResource
                */
                SoftSensorResource();

                /**
                * Virtual destructor for SoftSensorResource
                */
                virtual ~SoftSensorResource();

                /**
                * Initialize input and output attributes for the resource
                *
                * @return void
                */
                virtual void initAttributes();

                /**
                * This function should be implemented by the according bundle resource
                * and execute the according business logic (e.g., light switch or sensor resource)
                * to retrieve a sensor value. If a new sensor value is retrieved, the
                * setAttribute data should be called to update the value.
                * The implementor of the function can decide weather to notify OIC clients
                * about the changed state or not.
                *
                * @return Value of all attributes
                */
                virtual RCSResourceAttributes &handleGetAttributesRequest() = 0;

                /**
                * This function should be implemented by the according bundle resource
                * and execute the according business logic (e.g., light switch or sensor resource)
                * and write either on soft sensor values or external bridged devices.
                *
                * The call of this method could for example trigger a HTTP PUT request on
                * an external APIs. This method is responsible to update the resource internal
                * data and call the setAttribute method.
                *
                * The implementor of the function can decide weather to notify OIC clients
                * about the changed state or not.
                *
                * @param attrs Attributes to set
                *
                * @return void
                */
                virtual void handleSetAttributesRequest(RCSResourceAttributes &attrs) = 0;

                /**
                * SoftSensor logic. Has to be provided by the soft sensor developer.
                * This function will be executed if an input attribute is updated.
                *
                * @return void
                */
                virtual void executeLogic() = 0;

                /**
                * Callback from the client module in the container.
                * This function will be called if input data from remote resources are updated.
                * SoftSensor resource can get a vector of input data from multiple input resources
                *    which have attributeName that softsensor needs to execute its logic.
                *
                * @param attributeName Attribute key of input data
                *
                * @param values Vector of input data value
                *
                * @return void
                */
                virtual void onUpdatedInputResource(const std::string attributeName,
                                                    std::vector<RCSResourceAttributes::Value> values) = 0;


            public:
                std::list<std::string> m_inputList;
        };
    }
}

#endif
