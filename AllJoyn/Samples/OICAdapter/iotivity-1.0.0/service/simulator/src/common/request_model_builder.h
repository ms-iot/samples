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

#ifndef REQUEST_MODEL_BUILDER_H_
#define REQUEST_MODEL_BUILDER_H_

#include "request_model.h"
#include "response_model.h"

class RequestModelBuilder
{
    public:
        RequestModelBuilder(std::shared_ptr<RAML::Raml> &raml);
        std::map<RequestType, RequestModelSP> build(const std::string &uri);

    private:
        RequestModelSP createRequestModel(const RAML::ActionPtr &action);
        ResponseModelSP createResponseModel(int code, const RAML::ResponsePtr &response);
        SimulatorResourceModelSP createRepSchema(const RAML::RequestResponseBodyPtr &rep);
        RequestType getRequestType(RAML::ActionType actionType);

        std::shared_ptr<RAML::Raml> m_raml;
};

#endif

