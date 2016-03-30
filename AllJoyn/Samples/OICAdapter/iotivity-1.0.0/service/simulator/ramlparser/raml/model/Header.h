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

/**
 * @file   Header.h
 *
 * @brief   This file provides data Model for RAML Header.
 */

#ifndef HEADER_PARAMETER_H
#define HEADER_PARAMETER_H

#include "AbstractParam.h"

namespace RAML
{
    /**
     * @class   Header
     * @brief   This class provides data Model for RAML Header.
     */
    class Header: public AbstractParam
    {
        public:
            /**
                   * Constructor of Header.
                   *
                   * @param yamlNode - Reference to YamlNode for reading the Header
                   *
                   */
            Header(const YAML::Node &yamlNode) : AbstractParam(yamlNode) {}

            /**
                  * Constructor of Header.
                  */
            Header() {}
    };

    /** HeaderPtr - shared Ptr to Header.*/
    typedef std::shared_ptr<Header> HeaderPtr;

}
#endif

