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

#ifndef REQUEST_MODEL_H_
#define REQUEST_MODEL_H_

#include "simulator_client_types.h"
#include "simulator_resource_model.h"
#include "response_model.h"
#include "Raml.h"

typedef std::map<std::string, std::vector<std::string>> SupportedQueryParams;

class RequestModelBuilder;
class RequestModel
{
    public:
        friend class RequestModelBuilder;

        RequestType type() const;
        SupportedQueryParams getQueryParams();
        std::vector<std::string> getQueryParams(const std::string &key);
        SimulatorResourceModelSP getRepSchema();
        SimulatorResult validateResponse(int responseCode, const OC::OCRepresentation &rep);

    private:
        RequestModel(RequestType type);
        void setQueryParams(const SupportedQueryParams &queryParams);
        void addQueryParams(const std::string &key, const std::vector<std::string> &values);
        void addQueryParam(const std::string &key, const std::string &value);
        void addResponseModel(int code, ResponseModelSP &responseModel);
        void setRepSchema(SimulatorResourceModelSP &repSchema);

        RequestType m_type;
        SupportedQueryParams m_queryParams;
        std::map<int, ResponseModelSP> m_responseList;
        SimulatorResourceModelSP m_repSchema;
};

typedef std::shared_ptr<RequestModel> RequestModelSP;

#endif
