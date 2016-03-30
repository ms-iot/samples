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

/**
 * @file
 *
 * This file contains the declaration of classes and its members related to
 * ResourceRequest.
 */

#ifndef __OCRESOURCEREQUEST_H
#define __OCRESOURCEREQUEST_H

#include "OCApi.h"
#include "OCRepresentation.h"

void formResourceRequest(OCEntityHandlerFlag,
                         OCEntityHandlerRequest*,
                         std::shared_ptr<OC::OCResourceRequest>);


namespace OC
{
    /**
    *   @brief  OCResourceRequest provides APIs to extract details from a request URI
    */
    class OCResourceRequest
    {
    public:
        typedef std::shared_ptr<OCResourceRequest> Ptr;

        OCResourceRequest():
            m_requestType{},
            m_resourceUri{},
            m_queryParameters{},
            m_requestHandlerFlag{},
            m_representation{},
            m_observationInfo{},
            m_headerOptions{},
            m_requestHandle{nullptr},
            m_resourceHandle{nullptr}
        {
        }

        OCResourceRequest(OCResourceRequest&&) = default;
        OCResourceRequest& operator=(OCResourceRequest&&) = default;
        /**
        *  Virtual destructor
        */
        virtual ~OCResourceRequest(void)
        {
        }

        /**
        *  Retrieves the type of request string for the entity handler function to operate
        *  @return std::string request type. This could be 'GET'/'PUT'/'POST'/'DELETE'
        */
        std::string getRequestType() const {return m_requestType;}

        /**
        *  Retrieves the query parameters from the request
        *  @return std::string query parameters in the request
        */
        const QueryParamsMap& getQueryParameters() const {return m_queryParameters;}

        /**
        *  Retrieves the request handler flag type. This can be either INIT flag or
        *  REQUEST flag or OBSERVE flag.
        *  NOTE:
        *  INIT indicates that the vendor's entity handler should go and perform
        *  initialization operations
        *  REQUEST indicates that it is a request of certain type (GET/PUT/POST/DELETE)
        *  and entity handler needs to perform corresponding operations
        *  OBSERVE indicates that the request is of type Observe and entity handler
        *  needs to perform corresponding operations
        *  @return int type of request flag
        */
        int getRequestHandlerFlag() const {return m_requestHandlerFlag;}

        /**
        *  Provides the entire resource attribute representation
        *  @return OCRepresentation reference containing the name value pairs
        *   representing the resource's attributes
        */
        const OCRepresentation& getResourceRepresentation() const {return m_representation;}

        /**
        *  @return ObservationInfo reference provides observation information
        */
        const ObservationInfo& getObservationInfo() const {return m_observationInfo;}

        /**
        *  sets resource uri
        *  @param resourceUri specifies the resource uri
        */
        void setResourceUri(const std::string resourceUri)
        {
            m_resourceUri = resourceUri;
        }

        /**
        *  gets resource uri
        *  @return std::string resource uri
        */
        std::string getResourceUri(void)
        {
            return m_resourceUri;
        }

        /**
        * This API retrieves headerOptions which was sent from a client
        *
        * @return std::map HeaderOptions with the header options
        */
        const HeaderOptions& getHeaderOptions() const
        {
            return m_headerOptions;
        }

        /**
        * This API retrieves the request handle
        *
        * @return OCRequestHandle
        */
        const OCRequestHandle& getRequestHandle() const
        {
            return m_requestHandle;
        }

        /**
        * This API retrieves the resource handle
        *
        * return OCResourceHandle
        */
        const OCResourceHandle& getResourceHandle() const
        {
            return m_resourceHandle;
        }

    private:
        std::string m_requestType;
        std::string m_resourceUri;
        QueryParamsMap m_queryParameters;
        int m_requestHandlerFlag;
        OCRepresentation m_representation;
        ObservationInfo m_observationInfo;
        HeaderOptions m_headerOptions;
        OCRequestHandle m_requestHandle;
        OCResourceHandle m_resourceHandle;


    private:
        friend void (::formResourceRequest)(OCEntityHandlerFlag, OCEntityHandlerRequest*,
            std::shared_ptr<OC::OCResourceRequest>);
        void setRequestType(const std::string& requestType)
        {
            m_requestType = requestType;
        }

        void setPayload(OCPayload* requestPayload);

        void setQueryParams(QueryParamsMap& queryParams)
        {
            m_queryParameters = queryParams;
        }

        void setRequestHandlerFlag(int requestHandlerFlag)
        {
            m_requestHandlerFlag = requestHandlerFlag;
        }

        void setObservationInfo(const ObservationInfo& observationInfo)
        {
            m_observationInfo = observationInfo;
        }

        void setHeaderOptions(const HeaderOptions& headerOptions)
        {
            m_headerOptions = headerOptions;
        }

        /**
        * This API allows to set request handle
        * @param requestHandle - OCRequestHandle type used to set the
        * request handle
        */
        void setRequestHandle(const OCRequestHandle& requestHandle)
        {
            m_requestHandle = requestHandle;
        }

        /**
        * This API allows to set the resource handle
        * @param resourceHandle - OCResourceHandle type used to set the
        * resource handle
        */
        void setResourceHandle(const OCResourceHandle& resourceHandle)
        {
            m_resourceHandle = resourceHandle;
        }

    };
 }// namespace OC

#endif //__OCRESOURCEREQUEST_H
