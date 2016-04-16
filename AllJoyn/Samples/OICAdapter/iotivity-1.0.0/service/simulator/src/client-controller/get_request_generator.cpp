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

#include "get_request_generator.h"
#include "simulator_resource_model.h"
#include "logger.h"

#define TAG "GET_REQUEST_GEN"

GETRequestGenerator::GETRequestGenerator(int id, RequestSenderSP &requestSender,
        AutoRequestGeneration::ProgressStateCallback callback)
    :   AutoRequestGeneration(RequestType::RQ_TYPE_GET, id, requestSender, callback),
        m_status(false),
        m_stopRequested(false) {}

GETRequestGenerator::GETRequestGenerator(int id, RequestSenderSP &requestSender,
        const std::map<std::string, std::vector<std::string>> &queryParams,
        AutoRequestGeneration::ProgressStateCallback callback)
    :   AutoRequestGeneration(RequestType::RQ_TYPE_GET, id, requestSender, callback),
        m_queryParamGen(queryParams),
        m_status(false),
        m_stopRequested(false) {}

void GETRequestGenerator::startSending()
{
    // Check if the operation is already in progress
    std::lock_guard<std::mutex> lock(m_statusLock);
    if (m_status)
    {
        OC_LOG(ERROR, TAG, "Operation already in progress !");
        throw OperationInProgressException("Another GET request generation session is already in progress!");
    }

    // Create thread and start sending requests in dispatched thread
    m_thread = std::make_shared<std::thread>(&GETRequestGenerator::SendAllRequests, this);
    m_status = true;
    m_thread->detach();
}

void GETRequestGenerator::stopSending()
{
    m_stopRequested = true;
}

void GETRequestGenerator::SendAllRequests()
{
    // Notify the progress status
    OC_LOG(DEBUG, TAG, "Sending OP_START event");
    m_callback(m_id, OP_START);

    do
    {
        if (!m_stopRequested)
        {
            // Get the next possible queryParameter
            std::map<std::string, std::string> queryParam = m_queryParamGen.next();

            // Send the request
            try
            {
                m_requestSender->sendRequest(queryParam, std::bind(&GETRequestGenerator::onResponseReceived, this,
                                             std::placeholders::_1, std::placeholders::_2), true);
                m_requestCnt++;
            }
            catch (SimulatorException &e)
            {
                m_stopRequested = true;
                return completed();
            }
        }
    }
    while (m_queryParamGen.hasNext());

    m_requestsSent = true;
    completed();
}

void GETRequestGenerator::onResponseReceived(SimulatorResult result,
        SimulatorResourceModelSP repModel)
{
    OC_LOG_V(INFO, TAG, "Response recieved result:%d", result);
    m_responseCnt++;
    completed();
}

void GETRequestGenerator::completed()
{
    if (m_requestCnt == m_responseCnt)
    {
        if (m_stopRequested)
        {
            OC_LOG(DEBUG, TAG, "Sending OP_ABORT event");
            m_callback(m_id, OP_ABORT);
        }
        else
        {
            OC_LOG(DEBUG, TAG, "Sending OP_COMPLETE event");
            m_callback(m_id, OP_COMPLETE);
        }
    }
}