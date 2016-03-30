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

#include "simulator_remote_resource_impl.h"
#include "request_model_builder.h"
#include "simulator_exceptions.h"
#include "logger.h"

#define TAG "SIMULATOR_REMOTE_RESOURCE"

SimulatorRemoteResourceImpl::SimulatorRemoteResourceImpl(std::shared_ptr<OC::OCResource>
        &ocResource)
    :   m_observeState(false),
        m_getRequestSender(new GETRequestSender(ocResource)),
        m_putRequestSender(new PUTRequestSender(ocResource)),
        m_postRequestSender(new POSTRequestSender(ocResource)),
        m_autoRequestGenMngr(nullptr),
        m_ocResource(ocResource)
{
    m_id = m_ocResource->sid().append(m_ocResource->uri());
}

std::string SimulatorRemoteResourceImpl::getURI() const
{
    return m_ocResource->uri();
}

std::string SimulatorRemoteResourceImpl::getHost() const
{
    return m_ocResource->host();
}

std::string SimulatorRemoteResourceImpl::getID() const
{
    return m_id;
}

SimulatorConnectivityType SimulatorRemoteResourceImpl::getConnectivityType() const
{
    return convertConnectivityType(m_ocResource->connectivityType());
}

std::vector < std::string > SimulatorRemoteResourceImpl::getResourceTypes() const
{
    return m_ocResource->getResourceTypes();
}

std::vector < std::string > SimulatorRemoteResourceImpl::getResourceInterfaces() const
{
    return m_ocResource->getResourceInterfaces();
}

bool SimulatorRemoteResourceImpl::isObservable() const
{
    return m_ocResource->isObservable();
}

void SimulatorRemoteResourceImpl::observe(ObserveType type,
        ObserveNotificationCallback callback)
{
    if (!callback)
    {
        OC_LOG(ERROR, TAG, "Invalid callback!");
        throw InvalidArgsException(SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
    }

    std::lock_guard<std::mutex> lock(m_observeMutex);
    if (m_observeState)
    {
        OC_LOG(WARNING, TAG, "Resource already in observe state !");
        throw SimulatorException(SIMULATOR_ERROR, "Resource is already being observed!");
    }

    OC::ObserveCallback observeCallback = [this, callback](const OC::HeaderOptions & headerOptions,
                                          const OC::OCRepresentation & rep, const int errorCode,
                                          const int sequenceNum)
    {
        // Convert OCRepresentation to SimulatorResourceModel
        SimulatorResourceModelSP repModel = SimulatorResourceModel::create(rep);
        callback(m_id, static_cast<SimulatorResult>(errorCode), repModel, sequenceNum);
    };

    OC::ObserveType observeType = OC::ObserveType::Observe;
    if (type == ObserveType::OBSERVE_ALL)
    {
        observeType = OC::ObserveType::ObserveAll;
    }

    try
    {
        OCStackResult ocResult = m_ocResource->observe(observeType, OC::QueryParamsMap(), observeCallback);
        if (OC_STACK_OK != ocResult)
        {
            throw SimulatorException(static_cast<SimulatorResult>(ocResult), OC::OCException::reason(ocResult));
        }
    }
    catch (OC::OCException &e)
    {
        throw SimulatorException(static_cast<SimulatorResult>(e.code()), e.reason());
    }

    m_observeState = true;
}

void SimulatorRemoteResourceImpl::cancelObserve()
{
    try
    {
        OCStackResult ocResult = m_ocResource->cancelObserve(OC::QualityOfService::HighQos);
        if (OC_STACK_OK != ocResult)
        {
            throw SimulatorException(static_cast<SimulatorResult>(ocResult), OC::OCException::reason(ocResult));
        }
    }
    catch (OC::OCException &e)
    {
        throw SimulatorException(static_cast<SimulatorResult>(e.code()), e.reason());
    }

    std::lock_guard<std::mutex> lock(m_observeMutex);
    m_observeState = false;
}

void SimulatorRemoteResourceImpl::get(const std::string &interfaceType,
                                      const std::map<std::string, std::string> &queryParams,
                                      ResponseCallback callback)
{
    if (!callback)
    {
        OC_LOG(ERROR, TAG, "Invalid callback!");
        throw InvalidArgsException(SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
    }

    if (!m_getRequestSender)
    {
        OC_LOG(ERROR, TAG, "Invalid GET request sender!");
        throw NoSupportException("Can not send GET request on this resource!");
    }

    m_getRequestSender->sendRequest(interfaceType, queryParams,
                                    nullptr, std::bind(
                                        &SimulatorRemoteResourceImpl::onResponseReceived,
                                        this, std::placeholders::_1, std::placeholders::_2, callback));
}

void SimulatorRemoteResourceImpl::get(const std::map<std::string, std::string> &queryParams,
                                      ResponseCallback callback)
{
    if (!callback)
    {
        OC_LOG(ERROR, TAG, "Invalid callback!");
        throw InvalidArgsException(SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
    }

    if (!m_getRequestSender)
    {
        OC_LOG(ERROR, TAG, "Invalid GET request sender !");
        throw NoSupportException("Can not send GET request on this resource!");
    }

    m_getRequestSender->sendRequest(std::string(), queryParams,
                                    nullptr, std::bind(
                                        &SimulatorRemoteResourceImpl::onResponseReceived,
                                        this, std::placeholders::_1, std::placeholders::_2, callback));
}

void SimulatorRemoteResourceImpl::put(const std::string &interfaceType,
                                      const std::map<std::string, std::string> &queryParams,
                                      SimulatorResourceModelSP representation,
                                      ResponseCallback callback)
{
    if (!callback)
    {
        OC_LOG(ERROR, TAG, "Invalid callback!");
        throw InvalidArgsException(SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
    }

    if (!m_putRequestSender)
    {
        OC_LOG(ERROR, TAG, "Invalid PUT request sender !");
        throw NoSupportException("Can not send PUT request on this resource!");
    }

    m_putRequestSender->sendRequest(interfaceType, queryParams,
                                    representation, std::bind(
                                        &SimulatorRemoteResourceImpl::onResponseReceived,
                                        this, std::placeholders::_1, std::placeholders::_2, callback));
}

void SimulatorRemoteResourceImpl::put(const std::map<std::string, std::string> &queryParams,
                                      SimulatorResourceModelSP representation,
                                      ResponseCallback callback)
{
    if (!callback)
    {
        OC_LOG(ERROR, TAG, "Invalid callback!");
        throw InvalidArgsException(SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
    }

    if (!m_putRequestSender)
    {
        OC_LOG(ERROR, TAG, "Invalid PUT request sender !");
        throw NoSupportException("Can not send PUT request on this resource!");
    }

    m_putRequestSender->sendRequest(std::string(), queryParams,
                                    representation, std::bind(
                                        &SimulatorRemoteResourceImpl::onResponseReceived,
                                        this, std::placeholders::_1, std::placeholders::_2, callback));
}

void SimulatorRemoteResourceImpl::post(const std::string &interfaceType,
                                       const std::map<std::string, std::string> &queryParams,
                                       SimulatorResourceModelSP representation,
                                       ResponseCallback callback)
{
    if (!callback)
    {
        OC_LOG(ERROR, TAG, "Invalid callback!");
        throw InvalidArgsException(SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
    }

    if (!m_postRequestSender)
    {
        OC_LOG(ERROR, TAG, "Invalid POST request sender !");
        throw NoSupportException("Can not send POST request on this resource!");
    }

    m_postRequestSender->sendRequest(interfaceType, queryParams,
                                     representation, std::bind(
                                         &SimulatorRemoteResourceImpl::onResponseReceived,
                                         this, std::placeholders::_1, std::placeholders::_2, callback));
}

void SimulatorRemoteResourceImpl::post(const std::map<std::string, std::string> &queryParams,
                                       SimulatorResourceModelSP representation,
                                       ResponseCallback callback)
{
    if (!callback)
    {
        OC_LOG(ERROR, TAG, "Invalid callback!");
        throw InvalidArgsException(SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
    }

    if (!m_postRequestSender)
    {
        OC_LOG(ERROR, TAG, "Invalid POST request sender !");
        throw NoSupportException("Can not send POST request on this resource!");
    }

    m_postRequestSender->sendRequest(std::string(), queryParams,
                                     representation, std::bind(
                                         &SimulatorRemoteResourceImpl::onResponseReceived,
                                         this, std::placeholders::_1, std::placeholders::_2, callback));
}

int SimulatorRemoteResourceImpl::startVerification(RequestType type,
        StateCallback callback)
{
    if (!callback)
    {
        OC_LOG(ERROR, TAG, "Invalid callback!");
        throw InvalidArgsException(SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
    }

    if (!m_autoRequestGenMngr)
    {
        OC_LOG(ERROR, TAG, "Invalid auto request generation manager !");
        throw NoSupportException("Resource is not configured with RAML!");
    }

    if (m_requestModelList.end() == m_requestModelList.find(type))
        throw NoSupportException("Resource does not support this request type!");

    // Local callback for handling progress sate callback
    AutoRequestGeneration::ProgressStateCallback localCallback = [this, callback](
                int sessionId, OperationState state)
    {
        callback(m_id, sessionId, state);
    };

    switch (type)
    {
        case RequestType::RQ_TYPE_GET:
            if (m_getRequestSender)
            {
                return m_autoRequestGenMngr->startOnGET(m_getRequestSender,
                                                        m_requestModelList[RequestType::RQ_TYPE_GET]->getQueryParams(),
                                                        localCallback);
            }
            break;

        case RequestType::RQ_TYPE_PUT:
            if (m_putRequestSender)
            {
                return m_autoRequestGenMngr->startOnPUT(m_putRequestSender,
                                                        m_requestModelList[RequestType::RQ_TYPE_PUT]->getQueryParams(),
                                                        m_requestModelList[RequestType::RQ_TYPE_PUT]->getRepSchema(),
                                                        localCallback);
            }
            break;

        case RequestType::RQ_TYPE_POST:
            if (m_postRequestSender)
            {
                return m_autoRequestGenMngr->startOnPOST(m_putRequestSender,
                        m_requestModelList[RequestType::RQ_TYPE_POST]->getQueryParams(),
                        m_requestModelList[RequestType::RQ_TYPE_POST]->getRepSchema(),
                        localCallback);
            }
            break;

        case RequestType::RQ_TYPE_DELETE:
        default:
            throw NoSupportException("Resource does not support this request type!");
    }

    return -1; // Code should not reach here
}

void SimulatorRemoteResourceImpl::stopVerification(int id)
{
    if (id < 0)
    {
        OC_LOG(ERROR, TAG, "Invalid session id!");
        throw InvalidArgsException(SIMULATOR_INVALID_PARAM, "Invalid ID!");
    }

    if (!m_autoRequestGenMngr)
    {
        OC_LOG(ERROR, TAG, "Invalid auto request generation manager !");
        throw NoSupportException("Resource is not configured with RAML!");
    }

    m_autoRequestGenMngr->stop(id);
}

void SimulatorRemoteResourceImpl::configure(const std::string &path)
{
    if (path.empty())
    {
        OC_LOG(ERROR, TAG, "Invalid path given for configuration !");
        throw InvalidArgsException(SIMULATOR_INVALID_PARAM, "Empty path string!");
    }

    std::shared_ptr<RAML::RamlParser> ramlParser = std::make_shared<RAML::RamlParser>(path);
    RAML::RamlPtr raml = ramlParser->getRamlPtr();

    configure(raml);
}

void SimulatorRemoteResourceImpl::configure(std::shared_ptr<RAML::Raml> &raml)
{
    m_requestModelList = RequestModelBuilder(raml).build(m_ocResource->uri());
    if (m_getRequestSender &&
        m_requestModelList.end() != m_requestModelList.find(RequestType::RQ_TYPE_GET))
    {
        m_getRequestSender->setRequestModel(m_requestModelList[RequestType::RQ_TYPE_GET]);
    }

    if (m_putRequestSender &&
        m_requestModelList.end() != m_requestModelList.find(RequestType::RQ_TYPE_PUT))
    {
        m_putRequestSender->setRequestModel(m_requestModelList[RequestType::RQ_TYPE_PUT]);
    }

    if (m_postRequestSender &&
        m_requestModelList.end() != m_requestModelList.find(RequestType::RQ_TYPE_POST))
    {
        m_postRequestSender->setRequestModel(m_requestModelList[RequestType::RQ_TYPE_POST]);
    }

    if (!m_autoRequestGenMngr)
    {
        m_autoRequestGenMngr = std::make_shared<AutoRequestGenMngr>();
    }
}

void SimulatorRemoteResourceImpl::onResponseReceived(SimulatorResult result,
        SimulatorResourceModelSP repModel,
        ResponseCallback clientCallback)
{
    clientCallback(m_id, result, repModel);
}

SimulatorConnectivityType SimulatorRemoteResourceImpl::convertConnectivityType(
    OCConnectivityType type) const
{
    switch (type)
    {
        case CT_ADAPTER_IP:
            return SIMULATOR_CT_ADAPTER_IP;

        case CT_IP_USE_V4:
            return SIMULATOR_CT_IP_USE_V4 ;

        case CT_IP_USE_V6:
            return SIMULATOR_CT_IP_USE_V6;

        case CT_ADAPTER_GATT_BTLE:
            return SIMULATOR_CT_ADAPTER_GATT_BTLE;

        case CT_ADAPTER_RFCOMM_BTEDR:
            return SIMULATOR_CT_ADAPTER_RFCOMM_BTEDR;

        default:
            return SIMULATOR_CT_DEFAULT;
    }
}