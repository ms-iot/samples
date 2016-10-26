//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "OCResource.h"
#include "OCUtilities.h"

#include <boost/lexical_cast.hpp>
#include <sstream>

namespace OC {

static const char COAP[] = "coap://";
static const char COAPS[] = "coaps://";

#ifdef TCP_ADAPTER
static const char COAP_TCP[] = "coap+tcp://";
#endif

using OC::nil_guard;
using OC::result_guard;
using OC::checked_guard;

OCResource::OCResource(std::weak_ptr<IClientWrapper> clientWrapper,
                        const OCDevAddr& devAddr, const std::string& uri,
                        const std::string& serverId, bool observable,
                        const std::vector<std::string>& resourceTypes,
                        const std::vector<std::string>& interfaces)
 :  m_clientWrapper(clientWrapper), m_uri(uri),
    m_resourceId(serverId, m_uri), m_devAddr(devAddr),
    m_isObservable(observable), m_isCollection(false),
    m_resourceTypes(resourceTypes), m_interfaces(interfaces),
    m_observeHandle(nullptr)
{
    m_isCollection = std::find(m_interfaces.begin(), m_interfaces.end(), LINK_INTERFACE)
                        != m_interfaces.end();

    if (m_uri.empty() ||
        resourceTypes.empty() ||
        interfaces.empty()||
        m_clientWrapper.expired())
    {
        throw ResourceInitException(m_uri.empty(), resourceTypes.empty(),
                interfaces.empty(), m_clientWrapper.expired(), false, false);
    }
}

OCResource::OCResource(std::weak_ptr<IClientWrapper> clientWrapper,
                        const std::string& host, const std::string& uri,
                        const std::string& serverId,
                        OCConnectivityType connectivityType, bool observable,
                        const std::vector<std::string>& resourceTypes,
                        const std::vector<std::string>& interfaces)
 :  m_clientWrapper(clientWrapper), m_uri(uri),
    m_resourceId(serverId, m_uri),
    m_devAddr{ OC_DEFAULT_ADAPTER, OC_DEFAULT_FLAGS, 0, {0}, 0 },
    m_isObservable(observable), m_isCollection(false),
    m_resourceTypes(resourceTypes), m_interfaces(interfaces),
    m_observeHandle(nullptr)
{
    m_isCollection = std::find(m_interfaces.begin(), m_interfaces.end(), LINK_INTERFACE)
                        != m_interfaces.end();

    if (m_uri.empty() ||
        resourceTypes.empty() ||
        interfaces.empty()||
        m_clientWrapper.expired())
    {
        throw ResourceInitException(m_uri.empty(), resourceTypes.empty(),
                interfaces.empty(), m_clientWrapper.expired(), false, false);
    }

    if (uri.length() == 1 && uri[0] == '/')
    {
        throw ResourceInitException(m_uri.empty(), resourceTypes.empty(),
                interfaces.empty(), m_clientWrapper.expired(), false, false);
    }

    if (uri[0] != '/')
    {
        throw ResourceInitException(m_uri.empty(), resourceTypes.empty(),
                interfaces.empty(), m_clientWrapper.expired(), false, false);
    }

    // construct the devAddr from the pieces we have
    m_devAddr.adapter = static_cast<OCTransportAdapter>(connectivityType >> CT_ADAPTER_SHIFT);
    m_devAddr.flags = static_cast<OCTransportFlags>(connectivityType & CT_MASK_FLAGS);
    size_t len = host.length();
    if (len >= MAX_ADDR_STR_SIZE)
    {
        throw std::length_error("host address is too long.");
    }

    this->setHost(host);
}

OCResource::~OCResource()
{
}

void OCResource::setHost(const std::string& host)
{
    size_t prefix_len;

    if(host.compare(0, sizeof(COAP) - 1, COAP) == 0)
    {
        prefix_len = sizeof(COAP) - 1;
    }
    else if(host.compare(0, sizeof(COAPS) - 1, COAPS) == 0)
    {
        prefix_len = sizeof(COAPS) - 1;
        m_devAddr.flags = static_cast<OCTransportFlags>(m_devAddr.flags & OC_SECURE);
    }
#ifdef TCP_ADAPTER
    else if (host.compare(0, sizeof(COAP_TCP) - 1, COAP_TCP) == 0)
    {
        prefix_len = sizeof(COAP_TCP) - 1;
        m_devAddr.adapter = static_cast<OCTransportAdapter>(m_devAddr.adapter & OC_ADAPTER_TCP);
    }
#endif
    else
    {
        throw ResourceInitException(m_uri.empty(), m_resourceTypes.empty(),
            m_interfaces.empty(), m_clientWrapper.expired(), false, false);
    }

    // removed coap:// or coaps:// or coap+tcp://
    std::string host_token = host.substr(prefix_len);

    if(host_token[0] == '[')
    {
        m_devAddr.flags = static_cast<OCTransportFlags>(m_devAddr.flags & OC_IP_USE_V6);

        size_t found = host_token.find(']');

        if(found == std::string::npos || found == 0)
        {
            throw ResourceInitException(m_uri.empty(), m_resourceTypes.empty(),
                m_interfaces.empty(), m_clientWrapper.expired(), false, false);
        }
        // extract the ipaddress
        std::string ip6Addr = host_token.substr(1, found-1);
        ip6Addr.copy(m_devAddr.addr, sizeof(m_devAddr.addr));
        m_devAddr.addr[ip6Addr.length()] = '\0';
        //skip ']' and ':' characters in host string
        host_token = host_token.substr(found + 2);
    }
    else
    {
        size_t found = host_token.find(':');

        if(found == std::string::npos || found == 0)
        {
            throw ResourceInitException(m_uri.empty(), m_resourceTypes.empty(),
                m_interfaces.empty(), m_clientWrapper.expired(), false, false);
        }

        std::string addrPart = host_token.substr(0, found);
        addrPart.copy(m_devAddr.addr, sizeof(m_devAddr.addr));
        m_devAddr.addr[addrPart.length()] = '\0';
        //skip ':' character in host string
        host_token = host_token.substr(found + 1);
    }

    int port = std::stoi(host_token);

    if( port < 0 || port > UINT16_MAX )
    {
        throw ResourceInitException(m_uri.empty(), m_resourceTypes.empty(),
            m_interfaces.empty(), m_clientWrapper.expired(), false, false);
    }

    m_devAddr.port = static_cast<uint16_t>(port);

}

OCStackResult OCResource::get(const QueryParamsMap& queryParametersMap,
                              GetCallback attributeHandler, QualityOfService QoS)
{
    return checked_guard(m_clientWrapper.lock(),
                            &IClientWrapper::GetResourceRepresentation,
                            m_devAddr, m_uri,
                            queryParametersMap, m_headerOptions,
                            attributeHandler, QoS);
}

OCStackResult OCResource::get(const QueryParamsMap& queryParametersMap,
                              GetCallback attributeHandler)
{
    QualityOfService defaultQos = OC::QualityOfService::NaQos;
    checked_guard(m_clientWrapper.lock(), &IClientWrapper::GetDefaultQos, defaultQos);
    return result_guard(get(queryParametersMap, attributeHandler, defaultQos));
}

OCStackResult OCResource::get(const std::string& resourceType,
                     const std::string& resourceInterface, const QueryParamsMap& queryParametersMap,
                     GetCallback attributeHandler)
{
    QualityOfService defaultQoS = OC::QualityOfService::NaQos;
    checked_guard(m_clientWrapper.lock(), &IClientWrapper::GetDefaultQos, defaultQoS);

    return result_guard(get(resourceType, resourceInterface, queryParametersMap, attributeHandler, defaultQoS));
}

OCStackResult OCResource::get(const std::string& resourceType, const std::string& resourceInterface, const QueryParamsMap& queryParametersMap, GetCallback attributeHandler,
        QualityOfService QoS)
{
    QueryParamsMap mapCpy(queryParametersMap);

    if(!resourceType.empty())
    {
        mapCpy[OC::Key::RESOURCETYPESKEY]=resourceType;
    }

    if(!resourceInterface.empty())
    {
        mapCpy[OC::Key::INTERFACESKEY]= resourceInterface;
    }

    return result_guard(get(mapCpy, attributeHandler, QoS));
}

OCStackResult OCResource::put(const OCRepresentation& rep,
                              const QueryParamsMap& queryParametersMap, PutCallback attributeHandler,
                              QualityOfService QoS)
{
    return checked_guard(m_clientWrapper.lock(), &IClientWrapper::PutResourceRepresentation,
                         m_devAddr, m_uri, rep, queryParametersMap,
                         m_headerOptions, attributeHandler, QoS);
}

OCStackResult OCResource::put(const OCRepresentation& rep,
                              const QueryParamsMap& queryParametersMap, PutCallback attributeHandler)
{
    QualityOfService defaultQos = OC::QualityOfService::NaQos;
    checked_guard(m_clientWrapper.lock(), &IClientWrapper::GetDefaultQos, defaultQos);
    return result_guard(put(rep, queryParametersMap, attributeHandler, defaultQos));
}

OCStackResult OCResource::put(const std::string& resourceType,
                              const std::string& resourceInterface, const OCRepresentation& rep,
                              const QueryParamsMap& queryParametersMap,
                              PutCallback attributeHandler)
{
    QualityOfService defaultQos = OC::QualityOfService::NaQos;
    checked_guard(m_clientWrapper.lock(), &IClientWrapper::GetDefaultQos, defaultQos);

    return result_guard(put(resourceType, resourceInterface, rep, queryParametersMap,
            attributeHandler, defaultQos));
}

OCStackResult OCResource::put(const std::string& resourceType,
                              const std::string& resourceInterface, const OCRepresentation& rep,
                              const QueryParamsMap& queryParametersMap,
                              PutCallback attributeHandler,
                              QualityOfService QoS)
{
    QueryParamsMap mapCpy(queryParametersMap);

    if(!resourceType.empty())
    {
        mapCpy[OC::Key::RESOURCETYPESKEY]=resourceType;
    }

    if(!resourceInterface.empty())
    {
        mapCpy[OC::Key::INTERFACESKEY]=resourceInterface;
    }

    return result_guard(put(rep, mapCpy, attributeHandler, QoS));
}

OCStackResult OCResource::post(const OCRepresentation& rep,
                               const QueryParamsMap& queryParametersMap, PostCallback attributeHandler,
                               QualityOfService QoS)
{
    return checked_guard(m_clientWrapper.lock(), &IClientWrapper::PostResourceRepresentation,
                         m_devAddr, m_uri, rep, queryParametersMap,
                         m_headerOptions, attributeHandler, QoS);
}

OCStackResult OCResource::post(const OCRepresentation& rep,
                              const QueryParamsMap& queryParametersMap, PutCallback attributeHandler)
{
    QualityOfService defaultQos = OC::QualityOfService::NaQos;
    checked_guard(m_clientWrapper.lock(), &IClientWrapper::GetDefaultQos, defaultQos);
    return result_guard(post(rep, queryParametersMap, attributeHandler, defaultQos));
}

OCStackResult OCResource::post(const std::string& resourceType,
                               const std::string& resourceInterface, const OCRepresentation& rep,
                               const QueryParamsMap& queryParametersMap,
                               PostCallback attributeHandler)
{
    QualityOfService defaultQoS = OC::QualityOfService::NaQos;
    checked_guard(m_clientWrapper.lock(), &IClientWrapper::GetDefaultQos, defaultQoS);

    return result_guard(post(resourceType, resourceInterface, rep, queryParametersMap, attributeHandler,
            defaultQoS));
}

OCStackResult OCResource::post(const std::string& resourceType,
                               const std::string& resourceInterface, const OCRepresentation& rep,
                               const QueryParamsMap& queryParametersMap,
                               PostCallback attributeHandler,
                               QualityOfService QoS)
{
    QueryParamsMap mapCpy(queryParametersMap);

    if(!resourceType.empty())
    {
        mapCpy[OC::Key::RESOURCETYPESKEY]=resourceType;
    }

    if(!resourceInterface.empty())
    {
        mapCpy[OC::Key::INTERFACESKEY]=resourceInterface;
    }

    return result_guard(post(rep, mapCpy, attributeHandler, QoS));
}

OCStackResult OCResource::deleteResource(DeleteCallback deleteHandler, QualityOfService QoS)
{
    return checked_guard(m_clientWrapper.lock(), &IClientWrapper::DeleteResource,
                         m_devAddr, m_uri, m_headerOptions, deleteHandler, QoS);
}

OCStackResult OCResource::deleteResource(DeleteCallback deleteHandler)
{
    QualityOfService defaultQos = OC::QualityOfService::NaQos;
    checked_guard(m_clientWrapper.lock(), &IClientWrapper::GetDefaultQos, defaultQos);

    return result_guard(deleteResource(deleteHandler, defaultQos));
}

OCStackResult OCResource::observe(ObserveType observeType,
        const QueryParamsMap& queryParametersMap, ObserveCallback observeHandler,
        QualityOfService QoS)
{
    if(m_observeHandle != nullptr)
    {
        return result_guard(OC_STACK_INVALID_PARAM);
    }

    return checked_guard(m_clientWrapper.lock(), &IClientWrapper::ObserveResource,
                         observeType, &m_observeHandle, m_devAddr,
                         m_uri, queryParametersMap, m_headerOptions,
                         observeHandler, QoS);
}

OCStackResult OCResource::observe(ObserveType observeType,
        const QueryParamsMap& queryParametersMap, ObserveCallback observeHandler)
{
    QualityOfService defaultQoS = OC::QualityOfService::NaQos;
    checked_guard(m_clientWrapper.lock(), &IClientWrapper::GetDefaultQos, defaultQoS);

    return result_guard(observe(observeType, queryParametersMap, observeHandler, defaultQoS));
}

OCStackResult OCResource::cancelObserve()
{
    QualityOfService defaultQoS = OC::QualityOfService::NaQos;
    checked_guard(m_clientWrapper.lock(), &IClientWrapper::GetDefaultQos, defaultQoS);
    return result_guard(cancelObserve(defaultQoS));
}

OCStackResult OCResource::cancelObserve(QualityOfService QoS)
{
    if(m_observeHandle == nullptr)
    {
        return result_guard(OC_STACK_INVALID_PARAM);
    }

    OCStackResult result =  checked_guard(m_clientWrapper.lock(),
            &IClientWrapper::CancelObserveResource,
            m_observeHandle, "", m_uri, m_headerOptions, QoS);

    if(result == OC_STACK_OK)
    {
        m_observeHandle = nullptr;
    }

    return result;
}

void OCResource::setHeaderOptions(const HeaderOptions& headerOptions)
{
    m_headerOptions = headerOptions;
}

void OCResource::unsetHeaderOptions()
{
    m_headerOptions.clear();
}

std::string OCResource::host() const
{
    std::ostringstream ss;
    if (m_devAddr.flags & OC_SECURE)
    {
        ss << COAPS;
    }
#ifdef TCP_ADAPTER
    else if (m_devAddr.adapter & OC_ADAPTER_TCP)
    {
        ss << COAP_TCP;
    }
#endif
    else
    {
        ss << COAP;
    }
    if (m_devAddr.flags & OC_IP_USE_V6)
    {
        ss << '[' << m_devAddr.addr << ']';
    }
    else
    {
        ss << m_devAddr.addr;
    }
    if (m_devAddr.port)
    {
        ss << ':' << m_devAddr.port;
    }
    return ss.str();
}

std::string OCResource::uri() const
{
    return m_uri;
}

OCConnectivityType OCResource::connectivityType() const
{
    return static_cast<OCConnectivityType>(
           (m_devAddr.adapter << CT_ADAPTER_SHIFT) | (m_devAddr.flags & CT_MASK_FLAGS));
}

bool OCResource::isObservable() const
{
    return m_isObservable;
}

std::vector<std::string> OCResource::getResourceTypes() const
{
    return m_resourceTypes;
}

std::vector<std::string> OCResource::getResourceInterfaces(void) const
{
    return m_interfaces;
}

OCResourceIdentifier OCResource::uniqueIdentifier() const
{
    return m_resourceId;
}

std::string OCResource::sid() const
{
    return this->uniqueIdentifier().m_representation;
}

bool OCResource::operator==(const OCResource &other) const
{
    return m_resourceId == other.m_resourceId;
}

bool OCResource::operator!=(const OCResource &other) const
{
    return m_resourceId != other.m_resourceId;
}

bool OCResource::operator<(const OCResource &other) const
{
    return m_resourceId < other.m_resourceId;
}

bool OCResource::operator>(const OCResource &other) const
{
    return m_resourceId > other.m_resourceId;
}

bool OCResource::operator<=(const OCResource &other) const
{
    return m_resourceId <= other.m_resourceId;
}

bool OCResource::operator>=(const OCResource &other) const
{
    return m_resourceId >= other.m_resourceId;
}

OCResourceIdentifier::OCResourceIdentifier(const std::string& wireServerIdentifier,
        const std::string& resourceUri)
    :m_representation(wireServerIdentifier), m_resourceUri(resourceUri)
{
}

std::ostream& operator <<(std::ostream& os, const OCResourceIdentifier& ri)
{
    os << ri.m_representation<<ri.m_resourceUri;

    return os;
}

bool OCResourceIdentifier::operator==(const OCResourceIdentifier &other) const
{
    return m_representation == other.m_representation
        && m_resourceUri == other.m_resourceUri;
}

bool OCResourceIdentifier::operator!=(const OCResourceIdentifier &other) const
{
    return !(*this == other);
}

bool OCResourceIdentifier::operator<(const OCResourceIdentifier &other) const
{
    return m_resourceUri < other.m_resourceUri
        || (m_resourceUri == other.m_resourceUri &&
                m_representation < other.m_representation);
}

bool OCResourceIdentifier::operator>(const OCResourceIdentifier &other) const
{
    return *this != other && !(*this<other);
}

bool OCResourceIdentifier::operator<=(const OCResourceIdentifier &other) const
{
    return !(*this > other);
}

bool OCResourceIdentifier::operator>=(const OCResourceIdentifier &other) const
{
    return !(*this < other);
}

} // namespace OC

