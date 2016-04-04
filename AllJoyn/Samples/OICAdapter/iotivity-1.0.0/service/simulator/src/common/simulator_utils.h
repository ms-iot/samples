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
 * @file simulator_utils.h
 *
 * @brief This file provides utility methods used by both client and servers.
 */

#ifndef SIMULATOR_UTILS_H_
#define SIMULATOR_UTILS_H_

#include "simulator_exceptions.h"
#include "octypes.h"
#include "OCException.h"

#include <map>

/**
 * Utilities for Invokation of OC platfrom level APIs.
 */
template <typename FnT, typename... ArgsT>
typename std::enable_if<std::is_same<OCStackResult, decltype(std::declval<FnT>()(std::declval<ArgsT>()...))>::value>::type
invokeocplatform(FnT fn, ArgsT &&...args)
{
    try
    {
        OCStackResult ocResult = fn(std::forward<ArgsT>(args)...);
        if (OC_STACK_OK != ocResult)
        {
            // Throw SimulatorException converting error codes
            throw SimulatorException(static_cast<SimulatorResult>(ocResult), OC::OCException::reason(ocResult));
        }
    }
    catch (OC::OCException &e)
    {
        // Throw SimulatorException converting error codes
        throw SimulatorException(static_cast<SimulatorResult>(e.code()), e.reason());
    }
}

namespace OC
{
    class OCRepresentation;
}

std::string getPayloadString(const OC::OCRepresentation &);
std::string getRequestString(const std::map<std::string, std::string> &queryParams,
                             const OC::OCRepresentation &rep);
std::string getRequestString(const std::map<std::string, std::string> &queryParams);

#endif