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

#include "simulator_exceptions.h"

SimulatorException::SimulatorException(SimulatorResult errorCode, const std::string &message)
    : m_errorCode(errorCode), m_message(message) {}

const char *SimulatorException::what() const noexcept
{
    return m_message.c_str();
}

SimulatorResult SimulatorException::code() const
{
    return m_errorCode;
}

InvalidArgsException::InvalidArgsException(SimulatorResult errorCode, const std::string &message)
    : SimulatorException(errorCode, message) {}

NoSupportException::NoSupportException(const std::string &message)
    : SimulatorException(SIMULATOR_NOT_SUPPORTED, message) {}

OperationInProgressException::OperationInProgressException(const std::string &message)
    : SimulatorException(SIMULATOR_OPERATION_IN_PROGRESS, message) {}
