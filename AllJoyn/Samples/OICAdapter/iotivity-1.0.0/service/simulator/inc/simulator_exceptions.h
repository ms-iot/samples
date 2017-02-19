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
 * @file simulator_exceptions.h
 *
 * @brief This file provides exceptions which would be thrown from simualtor module.
 *
 */

#ifndef SIMULATOR_EXCEPTIONS_H_
#define SIMULATOR_EXCEPTIONS_H_

#include <exception>
#include "simulator_error_codes.h"

/**
 * @class SimulatorException
 * @brief This is the base exception of all type of exception thrown from simulator module.
 */
class SimulatorException : public std::exception
{
    public:
        /**
         * Constructor of SimulatorException.
         *
         * @param errorCode - Error code.
         * @param message - String describing the error messsage.
         */
        SimulatorException(SimulatorResult errorCode, const std::string &message);

        /**
         * API to get error message describing exception reason.
         *
         * @return Null terminated string.
         */
        virtual const char *what() const noexcept;

        /**
         * API to get error code with which exception is thrown.
         *
         * @return SimulatorResult - Error code.
         */
        virtual SimulatorResult code() const;
        virtual ~SimulatorException() throw() {}

    private:
        SimulatorResult m_errorCode;
        std::string m_message;
};

/**
 * @class InvalidArgsException
 * @brief This exception will be thrown to indicate invalid arguments case.
 */
class InvalidArgsException : public SimulatorException
{
    public:
        InvalidArgsException(SimulatorResult errorCode, const std::string &message);
};

/**
 * @class NoSupportException
 * @brief This exception will be thrown to indicate not supported operation cases.
 */
class NoSupportException : public SimulatorException
{
    public:
        NoSupportException(const std::string &message);
};

/**
 * @class OperationInProgressException
 * @brief This exception will be thrown to indicate requested operation is not allowed as other operation
 *              is in progress state.
 */
class OperationInProgressException : public SimulatorException
{
    public:
        OperationInProgressException(const std::string &message);
};

#endif
