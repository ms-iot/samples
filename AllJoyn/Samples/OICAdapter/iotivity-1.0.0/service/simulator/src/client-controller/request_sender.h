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

#ifndef REQUEST_SENDER_H_
#define REQUEST_SENDER_H_

#include "request_list.h"
#include "simulator_resource_model.h"
#include "request_model.h"
#include "simulator_exceptions.h"
#include "simulator_error_codes.h"

struct RequestDetail;
class RequestSender
{
    public:
        typedef std::function<void (SimulatorResult, SimulatorResourceModelSP)> ResponseCallback;

        RequestSender(RequestType type, std::shared_ptr<OC::OCResource> &ocResource);
        virtual ~RequestSender() {}

        void sendRequest(const std::map<std::string, std::string> &queryParam,
                         ResponseCallback responseCb, bool verifyResponse = false);

        void sendRequest(const std::string &interfaceType,
                         const std::map<std::string, std::string> &queryParam,
                         ResponseCallback responseCb, bool verifyResponse = false);

        void sendRequest(const std::map<std::string, std::string> &queryParam,
                         SimulatorResourceModelSP repModel,
                         ResponseCallback responseCb, bool verifyResponse = false);

        void sendRequest(const std::string &interfaceType,
                         const std::map<std::string, std::string> &queryParam,
                         SimulatorResourceModelSP repModel,
                         ResponseCallback responseCb, bool verifyResponse = false);

        void setRequestModel(const RequestModelSP &requestModel);

    protected:
        virtual OCStackResult send(OC::QueryParamsMap &queryParams,
                                   SimulatorResourceModelSP &repModel, OC::GetCallback callback) = 0;

        void onResponseReceived(const OC::HeaderOptions &headerOptions,
                                const OC::OCRepresentation &rep, const int errorCode, int requestId);

        RequestType m_type;
        RequestList<std::shared_ptr<RequestDetail>> m_requestList;
        RequestModelSP m_requestModel;
        std::shared_ptr<OC::OCResource> m_ocResource;
};

struct RequestDetail
{
    RequestType type;
    std::map<std::string, std::string> queryParam;
    SimulatorResourceModelSP body;
    bool verifyResponse;
    RequestSender::ResponseCallback responseCb;
};

typedef std::shared_ptr<RequestDetail> RequestDetailSP;

class GETRequestSender : public RequestSender
{
    public:
        GETRequestSender(std::shared_ptr<OC::OCResource> &ocResource);

        OCStackResult send(OC::QueryParamsMap &queryParams,
                           SimulatorResourceModelSP &repModel, OC::GetCallback callback);
};

class PUTRequestSender : public RequestSender
{
    public:
        PUTRequestSender(std::shared_ptr<OC::OCResource> &ocResource);

        OCStackResult send(OC::QueryParamsMap &queryParams,
                           SimulatorResourceModelSP &repModel, OC::GetCallback callback);
};

class POSTRequestSender : public RequestSender
{
    public:
        POSTRequestSender(std::shared_ptr<OC::OCResource> &ocResource);

        OCStackResult send(OC::QueryParamsMap &queryParams,
                           SimulatorResourceModelSP &repModel, OC::GetCallback callback);
};

typedef std::shared_ptr<RequestSender> RequestSenderSP;
typedef std::shared_ptr<GETRequestSender> GETRequestSenderSP;
typedef std::shared_ptr<PUTRequestSender> PUTRequestSenderSP;
typedef std::shared_ptr<POSTRequestSender> POSTRequestSenderSP;

#endif
