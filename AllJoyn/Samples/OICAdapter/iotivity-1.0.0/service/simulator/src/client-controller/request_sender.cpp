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

#include "request_sender.h"
#include "simulator_logger.h"
#include "simulator_utils.h"
#include "logger.h"

#define TAG "GET_REQUEST_SNDR"

RequestSender::RequestSender(RequestType type, std::shared_ptr<OC::OCResource> &ocResource)
    :   m_type(type), m_ocResource(ocResource) {}

void RequestSender::sendRequest(const std::map<std::string, std::string> &queryParam,
                                ResponseCallback responseCb, bool verifyResponse)
{
    sendRequest(std::string(), queryParam, nullptr, responseCb, verifyResponse);
}

void RequestSender::sendRequest(const std::string &interfaceType,
                                const std::map<std::string, std::string> &queryParam,
                                ResponseCallback responseCb, bool verifyResponse)
{
    sendRequest(interfaceType, queryParam, nullptr, responseCb, verifyResponse);
}

void RequestSender::sendRequest(const std::map<std::string, std::string> &queryParam,
                                SimulatorResourceModelSP repModel,
                                ResponseCallback responseCb, bool verifyResponse)
{
    sendRequest(std::string(), queryParam, repModel, responseCb, verifyResponse);
}

void RequestSender::sendRequest(const std::string &interfaceType,
                                const std::map<std::string, std::string> &queryParam,
                                SimulatorResourceModelSP repModel,
                                ResponseCallback responseCb, bool verifyResponse)
{
    // Add query paramter "if" if interfaceType is not empty
    OC::QueryParamsMap queryParamCpy(queryParam);
    if (!interfaceType.empty())
        queryParamCpy["if"] = interfaceType;

    // Add the request into request list
    RequestDetailSP requestDetail(new RequestDetail);
    requestDetail->type = m_type;
    requestDetail->queryParam = queryParamCpy;
    requestDetail->body = repModel;
    requestDetail->verifyResponse = verifyResponse;
    requestDetail->responseCb = responseCb;

    int requestId = m_requestList.add(requestDetail);

    OCStackResult ocResult = send(queryParamCpy, repModel, std::bind(
                                      &RequestSender::onResponseReceived, this,
                                      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, requestId));
    if (OC_STACK_OK != ocResult)
    {
        OC_LOG_V(ERROR, TAG, "Sending request failed [errorcode: %d]", ocResult);
        m_requestList.remove(requestId);
        throw SimulatorException(static_cast<SimulatorResult>(ocResult), "Failed to send request!");
    }
}

void RequestSender::setRequestModel(const RequestModelSP &requestModel)
{
    m_requestModel = requestModel;
}

void RequestSender::onResponseReceived(const OC::HeaderOptions &headerOptions,
                                       const OC::OCRepresentation &rep, const int errorCode, int requestId)
{
    SIM_LOG(ILogger::INFO, "Response recieved..." << "\n" << getPayloadString(rep));

    // Ignore the response recieved for invalid requests
    RequestDetailSP request = m_requestList.remove(requestId);
    if (!request)
    {
        return;
    }

    // Validate the response as per the schema given by RAML
    ValidationStatus validationStatus {false, SIMULATOR_ERROR};
    if (request->verifyResponse && m_requestModel
        && !errorCode) // TODO: Validate responses other than "200"
    {
        validationStatus.errorCode = m_requestModel->validateResponse(200, rep);
        validationStatus.isVerified = true;
    }

    SimulatorResourceModelSP repModel = SimulatorResourceModel::create(rep);
    request->responseCb(static_cast<SimulatorResult>(errorCode), repModel);
}

GETRequestSender::GETRequestSender(std::shared_ptr<OC::OCResource> &ocResource)
    :   RequestSender(RequestType::RQ_TYPE_GET, ocResource) {}

OCStackResult GETRequestSender::send(OC::QueryParamsMap &queryParams,
                                     SimulatorResourceModelSP &repModel, OC::GetCallback callback)
{
    SIM_LOG(ILogger::INFO, "Sending GET request..." << "\n" << getRequestString(queryParams));

    return m_ocResource->get(queryParams, callback);
}

PUTRequestSender::PUTRequestSender(std::shared_ptr<OC::OCResource> &ocResource)
    :   RequestSender(RequestType::RQ_TYPE_PUT, ocResource) {}

OCStackResult PUTRequestSender::send(OC::QueryParamsMap &queryParams,
                                     SimulatorResourceModelSP &repModel, OC::GetCallback callback)
{
    OC::OCRepresentation ocRep;
    if (repModel)
    {
        ocRep = repModel->getOCRepresentation();
    }

    SIM_LOG(ILogger::INFO, "Sending PUT request..." << "\n" << getRequestString(queryParams, ocRep));
    return m_ocResource->put(ocRep, queryParams, callback);
}

POSTRequestSender::POSTRequestSender(std::shared_ptr<OC::OCResource> &ocResource)
    :   RequestSender(RequestType::RQ_TYPE_POST, ocResource) {}

OCStackResult POSTRequestSender::send(OC::QueryParamsMap &queryParams,
                                      SimulatorResourceModelSP &repModel, OC::GetCallback callback)
{
    OC::OCRepresentation ocRep;
    if (repModel)
        ocRep = repModel->getOCRepresentation();

    SIM_LOG(ILogger::INFO, "Sending POST request..." << "\n" << getRequestString(queryParams, ocRep));
    return m_ocResource->post(ocRep, queryParams, callback);
}
