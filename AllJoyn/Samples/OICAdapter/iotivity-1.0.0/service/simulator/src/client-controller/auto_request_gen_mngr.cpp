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

#include "auto_request_gen_mngr.h"
#include "get_request_generator.h"
#include "put_request_generator.h"
#include "post_request_generator.h"
#include "simulator_exceptions.h"
#include "logger.h"

#define TAG "AUTO_REQ_GEN_MNGR"

int AutoRequestGenMngr::startOnGET(RequestSenderSP requestSender,
                                   const std::map<std::string, std::vector<std::string>> &queryParams,
                                   AutoRequestGeneration::ProgressStateCallback callback)
{
    // Input validation
    if (!requestSender)
    {
        throw InvalidArgsException(SIMULATOR_INVALID_PARAM, "Invalid request sender given!");
    }

    if (!callback)
    {
        throw InvalidArgsException(SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
    }

    // Check is there auto request generation session already going on for GET requests
    if (isInProgress(RequestType::RQ_TYPE_GET))
    {
        throw OperationInProgressException("Another GET request generation session is already in progress!");
    }

    // Create request generation session
    AutoRequestGeneration::ProgressStateCallback localCallback = std::bind(
                &AutoRequestGenMngr::onProgressChange, this,
                std::placeholders::_1, std::placeholders::_2, callback);

    std::lock_guard<std::mutex> lock(m_lock);
    std::shared_ptr<AutoRequestGeneration> requestGen(
        new GETRequestGenerator(m_id, requestSender, queryParams, localCallback));
    m_requestGenList[m_id] = requestGen;

    try
    {
        requestGen->start();
    }
    catch (OperationInProgressException &e)
    {
        m_requestGenList.erase(m_requestGenList.find(m_id));
        throw;
    }

    return m_id++;
}

int AutoRequestGenMngr::startOnPUT(RequestSenderSP requestSender,
                                   const std::map<std::string, std::vector<std::string>> &queryParams,
                                   SimulatorResourceModelSP resModel,
                                   AutoRequestGeneration::ProgressStateCallback callback)
{
    // Input validation
    if (!requestSender)
    {
        throw InvalidArgsException(SIMULATOR_INVALID_PARAM, "Invalid request sender given!");
    }

    if (!callback)
    {
        throw InvalidArgsException(SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
    }

    // Check is there auto request generation session already going on for GET requests
    if (isInProgress(RequestType::RQ_TYPE_PUT))
    {
        throw OperationInProgressException("Another GET request generation session is already in progress!");
    }

    // Create request generation session
    AutoRequestGeneration::ProgressStateCallback localCallback = std::bind(
                &AutoRequestGenMngr::onProgressChange, this,
                std::placeholders::_1, std::placeholders::_2, callback);

    // Create and make the entry in list
    std::lock_guard<std::mutex> lock(m_lock);
    std::shared_ptr<AutoRequestGeneration> requestGen(
        new PUTRequestGenerator(m_id, requestSender, queryParams, resModel, localCallback));
    m_requestGenList[m_id] = requestGen;

    try
    {
        requestGen->start();
    }
    catch (OperationInProgressException &e)
    {
        m_requestGenList.erase(m_requestGenList.find(m_id));
        throw;
    }

    return m_id++;
}

int AutoRequestGenMngr::startOnPOST(RequestSenderSP requestSender,
                                    const std::map<std::string, std::vector<std::string>> &queryParams,
                                    SimulatorResourceModelSP resModel,
                                    AutoRequestGeneration::ProgressStateCallback callback)
{
    // Input validation
    if (!requestSender)
    {
        throw InvalidArgsException(SIMULATOR_INVALID_PARAM, "Invalid request sender given!");
    }

    if (!callback)
    {
        throw InvalidArgsException(SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
    }

    // Check is there auto request generation session already going on for GET requests
    if (isInProgress(RequestType::RQ_TYPE_POST))
    {
        throw OperationInProgressException("Another GET request generation session is already in progress!");
    }

    // Create request generation session
    AutoRequestGeneration::ProgressStateCallback localCallback = std::bind(
                &AutoRequestGenMngr::onProgressChange, this,
                std::placeholders::_1, std::placeholders::_2, callback);

    // Create and make the entry in list
    std::lock_guard<std::mutex> lock(m_lock);
    std::shared_ptr<AutoRequestGeneration> requestGen(
        new POSTRequestGenerator(m_id, requestSender, queryParams, resModel, localCallback));
    m_requestGenList[m_id] = requestGen;

    try
    {
        requestGen->start();
    }
    catch (OperationInProgressException &e)
    {
        m_requestGenList.erase(m_requestGenList.find(m_id));
        throw;
    }

    return m_id++;
}

void AutoRequestGenMngr::stop(int id)
{
    std::lock_guard<std::mutex> lock(m_lock);
    if (m_requestGenList.end() != m_requestGenList.find(id))
    {
        m_requestGenList[id]->stop();
        OC_LOG_V(INFO, TAG, "Auto request generation session stopped [%d]", id);
        return;
    }

    OC_LOG_V(ERROR, TAG, "Invalid verification id : %d", id);
}

void AutoRequestGenMngr::onProgressChange(int sessionId, OperationState state,
        AutoRequestGeneration::ProgressStateCallback clientCallback)
{
    if (!isValid(sessionId))
        return;

    // Remove the request generator from list if it is completed
    if (state == OP_COMPLETE || state == OP_ABORT)
    {
        remove(sessionId);
    }

    // Delegate notification to app callback
    clientCallback(sessionId, state);
}

bool AutoRequestGenMngr::isValid(int id)
{
    std::lock_guard<std::mutex> lock(m_lock);
    if (m_requestGenList.end() != m_requestGenList.find(id))
    {
        return true;
    }

    return false;
}

bool AutoRequestGenMngr::isInProgress(RequestType type)
{
    std::lock_guard<std::mutex> lock(m_lock);
    for (auto & session : m_requestGenList)
    {
        if ((session.second)->type() == type)
            return true;
    }

    return false;
}

void AutoRequestGenMngr::remove(int id)
{
    std::lock_guard<std::mutex> lock(m_lock);
    if (m_requestGenList.end() != m_requestGenList.find(id))
    {
        m_requestGenList.erase(m_requestGenList.find(id));
    }
}
