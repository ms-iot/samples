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
 * @file   Helpers.h
 *
 * @brief   This file provides helper definitions for Json Schema parser.
 */

#ifndef HELPERS_H_
#define HELPERS_H_

#include <string>
#include <vector>
#include <map>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>

namespace RAML
{

    /** ValueVariant - Boost Variant to hold type of int, string, double and bool*/
    typedef boost::variant <
    int,
    double,
    bool,
    std::string
    > ValueVariant;

    /** VariantType - enumeration for variant types*/
    enum class VariantType
    {
        INT, DOUBLE, BOOL, STRING
    };


}
#endif
