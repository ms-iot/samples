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

#include "simulator_resource_server_impl.h"
#include "simulator_utils.h"
#include "simulator_logger.h"
#include "logger.h"

#define TAG "SIM_RESOURCE_SERVER"

SimulatorResourceServerImpl::SimulatorResourceServerImpl()
    :   m_resourceHandle(NULL)
{
    m_property = static_cast<OCResourceProperty>(OC_DISCOVERABLE | OC_OBSERVABLE);
    m_interfaceType.assign(OC::DEFAULT_INTERFACE);
}

bool SimulatorResourceServerImpl::isObservable() const
{
    return (m_property & OC_OBSERVABLE);
}

void SimulatorResourceServerImpl::setURI(const std::string &uri)
{
    m_uri = uri;
}

void SimulatorResourceServerImpl::setResourceType(const std::string &resourceType)
{
    m_resourceType = resourceType;
}

void SimulatorResourceServerImpl::setInterfaceType(const std::string &interfaceType)
{
    m_interfaceType = interfaceType;
}

void SimulatorResourceServerImpl::setName(const std::string &name)
{
    m_name = name;
}

void SimulatorResourceServerImpl::setObservable(bool state)
{
    if (true == state)
        m_property = static_cast<OCResourceProperty>(m_property | OC_OBSERVABLE);
    else
        m_property = static_cast<OCResourceProperty>(m_property ^ OC_OBSERVABLE);
}

int SimulatorResourceServerImpl::startUpdateAutomation(AutomationType type,
        updateCompleteCallback callback)
{
    if (!callback)
    {
        OC_LOG(ERROR, TAG, "Invalid callback!");
        throw InvalidArgsException(SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
    }

    if (!m_resourceHandle)
    {
        OC_LOG(ERROR, TAG, "Invalid resource!");
        throw SimulatorException(SIMULATOR_NO_RESOURCE, "Invalid resource!");
    }

    return m_updateAutomationMgr.startResourceAutomation(this, type, -1, callback);
}

int SimulatorResourceServerImpl::startUpdateAutomation(const std::string &attrName,
        AutomationType type,
        updateCompleteCallback callback)
{
    if (!callback)
    {
        OC_LOG(ERROR, TAG, "Invalid callback!");
        throw InvalidArgsException(SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
    }

    if (!m_resourceHandle)
    {
        OC_LOG(ERROR, TAG, "Invalid resource!");
        throw SimulatorException(SIMULATOR_NO_RESOURCE, "Invalid resource!");
    }

    return m_updateAutomationMgr.startAttributeAutomation(this, attrName, type, -1, callback);
}

std::vector<int> SimulatorResourceServerImpl::getResourceAutomationIds()
{
    return m_updateAutomationMgr.getResourceAutomationIds();
}

std::vector<int> SimulatorResourceServerImpl::getAttributeAutomationIds()
{
    return m_updateAutomationMgr.getAttributeAutomationIds();
}

void SimulatorResourceServerImpl::stopUpdateAutomation(const int id)
{
    m_updateAutomationMgr.stop(id);
}

void SimulatorResourceServerImpl::setModelChangeCallback(ResourceModelChangedCB callback)
{
    m_callback = callback;
}

void SimulatorResourceServerImpl::setObserverCallback(ObserverCB callback)
{
    m_observeCallback = callback;
}

std::vector<ObserverInfo> SimulatorResourceServerImpl::getObserversList()
{
    return m_observersList;
}

void SimulatorResourceServerImpl::notify(uint8_t id)
{
    if (!m_resourceHandle)
    {
        OC_LOG(ERROR, TAG, "Invalid resource!");
        throw SimulatorException(SIMULATOR_NO_RESOURCE, "Invalid resource!");
    }

    std::shared_ptr<OC::OCResourceResponse> resourceResponse =
    {std::make_shared<OC::OCResourceResponse>()};

    resourceResponse->setErrorCode(200);
    resourceResponse->setResponseResult(OC_EH_OK);
    resourceResponse->setResourceRepresentation(getOCRepresentation(), OC::DEFAULT_INTERFACE);

    OC::ObservationIds observers;
    observers.push_back(id);

    SIM_LOG(ILogger::INFO, "[" << m_uri << "] Sending notification to observer with id " << id);

    typedef OCStackResult (*NotifyListOfObservers)(OCResourceHandle, OC::ObservationIds &,
            const std::shared_ptr<OC::OCResourceResponse>);

    invokeocplatform(static_cast<NotifyListOfObservers>(OC::OCPlatform::notifyListOfObservers),
                     m_resourceHandle,
                     observers,
                     resourceResponse);
}

void SimulatorResourceServerImpl::notifyAll()
{
    if (!m_resourceHandle)
    {
        OC_LOG(ERROR, TAG, "Invalid resource!");
        throw SimulatorException(SIMULATOR_NO_RESOURCE, "Invalid resource!");
    }

    if (!m_observersList.size())
    {
        OC_LOG(ERROR, TAG, "Observers list is empty!");
        return;
    }

    std::shared_ptr<OC::OCResourceResponse> resourceResponse =
    {std::make_shared<OC::OCResourceResponse>()};

    resourceResponse->setErrorCode(200);
    resourceResponse->setResponseResult(OC_EH_OK);
    resourceResponse->setResourceRepresentation(getOCRepresentation(), OC::DEFAULT_INTERFACE);

    OC::ObservationIds observers;
    for (auto & observer : m_observersList)
        observers.push_back(observer.id);

    SIM_LOG(ILogger::INFO, "[" << m_uri << "] Sending notification to all observers");

    typedef OCStackResult (*NotifyListOfObservers)(OCResourceHandle, OC::ObservationIds &,
            const std::shared_ptr<OC::OCResourceResponse>);

    invokeocplatform(static_cast<NotifyListOfObservers>(OC::OCPlatform::notifyListOfObservers),
                     m_resourceHandle,
                     observers,
                     resourceResponse);
}

void SimulatorResourceServerImpl::start()
{
    if (m_uri.empty() || m_resourceType.empty() ||
        m_interfaceType.empty() || m_name.empty() || !m_callback)
    {
        OC_LOG(ERROR, TAG, "Invalid data found to register the resource!");
        throw InvalidArgsException(SIMULATOR_INVALID_PARAM, "Invalid data found to register the resource!");
    }

    if (m_resourceHandle)
    {
        OC_LOG(ERROR, TAG, "Resource already registered!");
        throw SimulatorException(SIMULATOR_ERROR, "Resource already registered!");
    }

    typedef OCStackResult (*RegisterResource)(OCResourceHandle &, std::string &, const std::string &,
            const std::string &, OC::EntityHandler, uint8_t);

    invokeocplatform(static_cast<RegisterResource>(OC::OCPlatform::registerResource),
                     m_resourceHandle, m_uri, m_resourceType, m_interfaceType,
                     std::bind(&SimulatorResourceServerImpl::entityHandler,
                               this, std::placeholders::_1), m_property);
}

void SimulatorResourceServerImpl::stop()
{
    if (!m_resourceHandle)
    {
        OC_LOG(ERROR, TAG, "Invalid resource!");
        throw SimulatorException(SIMULATOR_NO_RESOURCE, "Invalid resource!");
    }

    typedef OCStackResult (*UnregisterResource)(const OCResourceHandle &);

    invokeocplatform(static_cast<UnregisterResource>(OC::OCPlatform::unregisterResource),
                     m_resourceHandle);

    m_resourceHandle = nullptr;
}

void SimulatorResourceServerImpl::notifyApp()
{
    // Notify the application callback
    if (m_callback)
    {
        m_callback(m_uri, m_resModel);
    }
}

OC::OCRepresentation SimulatorResourceServerImpl::getOCRepresentation()
{
    return m_resModel.getOCRepresentation();
}

bool SimulatorResourceServerImpl::modifyResourceModel(OC::OCRepresentation &ocRep)
{
    bool status = m_resModel.update(ocRep);
    if (true == status)
    {
        resourceModified();
    }
    return status;
}

void SimulatorResourceServerImpl::resourceModified()
{
    if (!m_resourceHandle)
    {
        return;
    }

    // Notify all the subscribers
    notifyAll();

    // Notify the application callback
    if (m_callback)
    {
        m_callback(m_uri, m_resModel);
    }
}

OCEntityHandlerResult SimulatorResourceServerImpl::entityHandler(
    std::shared_ptr<OC::OCResourceRequest>
    request)
{
    OCEntityHandlerResult errCode = OC_EH_ERROR;
    if (!request)
    {
        return OC_EH_ERROR;
    }

    if (OC::RequestHandlerFlag::RequestFlag & request->getRequestHandlerFlag())
    {
        auto response = std::make_shared<OC::OCResourceResponse>();
        response->setRequestHandle(request->getRequestHandle());
        response->setResourceHandle(request->getResourceHandle());

        if ("GET" == request->getRequestType())
        {
            OC::OCRepresentation rep = request->getResourceRepresentation();
            std::string payload = getPayloadString(rep);
            SIM_LOG(ILogger::INFO, "[" << m_uri <<
                    "] GET request received. \n**Payload details**" << payload)

            response->setErrorCode(200);
            response->setResponseResult(OC_EH_OK);
            response->setResourceRepresentation(getOCRepresentation());

            if (OC_STACK_OK == OC::OCPlatform::sendResponse(response))
            {
                errCode = OC_EH_OK;
            }
        }
        else if ("PUT" == request->getRequestType())
        {
            OC::OCRepresentation rep = request->getResourceRepresentation();
            std::string payload = getPayloadString(rep);
            SIM_LOG(ILogger::INFO, "[" << m_uri <<
                    "] PUT request received. \n**Payload details**" << payload)

            if (true == modifyResourceModel(rep))
            {
                response->setErrorCode(200);
                response->setResponseResult(OC_EH_OK);
                response->setResourceRepresentation(getOCRepresentation());
            }
            else
            {
                response->setErrorCode(400);
                response->setResponseResult(OC_EH_ERROR);
            }

            if (OC_STACK_OK == OC::OCPlatform::sendResponse(response))
            {
                errCode = OC_EH_OK;
            }
        }
        else if ("POST" == request->getRequestType())
        {
            OC::OCRepresentation rep = request->getResourceRepresentation();
            std::string payload = getPayloadString(rep);
            SIM_LOG(ILogger::INFO, "[" << m_uri <<
                    "] POST request received. \n**Payload details**" << payload)

            if (true == modifyResourceModel(rep))
            {
                response->setErrorCode(200);
                response->setResponseResult(OC_EH_OK);
                response->setResourceRepresentation(getOCRepresentation());
            }
            else
            {
                response->setErrorCode(400);
                response->setResponseResult(OC_EH_ERROR);
            }

            if (OC_STACK_OK == OC::OCPlatform::sendResponse(response))
            {
                errCode = OC_EH_OK;
            }
        }
        else if ("DELETE" == request->getRequestType())
        {
            OC::OCRepresentation rep = request->getResourceRepresentation();
            std::string payload = getPayloadString(rep);
            SIM_LOG(ILogger::INFO, "[" << m_uri <<
                    "] DELETE request received. \n**Payload details**" << payload)

            // DELETE request handling not supported right now
            response->setErrorCode(400);
            response->setResponseResult(OC_EH_ERROR);
            if (OC_STACK_OK == OC::OCPlatform::sendResponse(response))
            {
                errCode = OC_EH_OK;
            }
        }
        else
        {
            OC::OCRepresentation rep = request->getResourceRepresentation();
            std::string payload = getPayloadString(rep);
            SIM_LOG(ILogger::INFO, "[" << m_uri <<
                    "] UNKNOWN type request received. \n**Payload details**" << payload)

            response->setResponseResult(OC_EH_ERROR);
            if (OC_STACK_OK == OC::OCPlatform::sendResponse(response))
            {
                errCode = OC_EH_ERROR;
            }
        }
    }

    if (OC::RequestHandlerFlag::ObserverFlag & request->getRequestHandlerFlag())
    {
        if (false == isObservable())
        {
            SIM_LOG(ILogger::INFO, "[" << m_uri << "] OBSERVE request received")
            SIM_LOG(ILogger::INFO, "[" << m_uri << "] Sending error as resource is in unobservable state")
            return OC_EH_ERROR;
        }

        OC::ObservationInfo observationInfo = request->getObservationInfo();
        if (OC::ObserveAction::ObserveRegister == observationInfo.action)
        {
            SIM_LOG(ILogger::INFO, "[" << m_uri << "] OBSERVE REGISTER request received");

            ObserverInfo info {observationInfo.obsId, observationInfo.address, observationInfo.port};
            m_observersList.push_back(info);

            //Inform about addition of observer
            if (m_observeCallback)
            {
                m_observeCallback(m_uri, ObservationStatus::OBSERVE_REGISTER, info);
            }
        }
        else if (OC::ObserveAction::ObserveUnregister == observationInfo.action)
        {
            SIM_LOG(ILogger::INFO, "[" << m_uri << "] OBSERVE UNREGISTER request received");

            ObserverInfo info;
            for (auto iter = m_observersList.begin(); iter != m_observersList.end(); iter++)
            {
                if ((info = *iter), info.id == observationInfo.obsId)
                {
                    m_observersList.erase(iter);
                    break;
                }
            }

            // Inform about cancellation of observer
            if (m_observeCallback)
            {
                m_observeCallback(m_uri, ObservationStatus::OBSERVE_UNREGISTER, info);
            }
        }
        errCode = OC_EH_OK;
    }

    return errCode;
}
