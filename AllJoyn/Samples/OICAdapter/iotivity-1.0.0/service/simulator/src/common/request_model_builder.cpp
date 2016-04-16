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

#include "request_model_builder.h"
#include "logger.h"

#define TAG "REQ_MODEL_BUILDER"

RequestModelBuilder::RequestModelBuilder(std::shared_ptr<RAML::Raml> &raml)
    : m_raml (raml) {}

std::map<RequestType, RequestModelSP> RequestModelBuilder::build(const std::string &uri)
{
    std::map<RequestType, RequestModelSP> modelList;
    if (!m_raml)
    {
        return modelList;
    }

    for (auto  & resource : m_raml->getResources())
    {
        // Pick the resource based on the resource uri.
        if (std::string::npos == uri.find((resource.second)->getResourceUri()))
            continue;

        // Construcut Request and Response Model from RAML::Action
        for (auto  & action :  (resource.second)->getActions())
        {
            RequestModelSP requestModel = createRequestModel(action.second);
            if (requestModel)
                modelList[requestModel->type()] = requestModel;
        }
    }

    return modelList;
}

RequestModelSP RequestModelBuilder::createRequestModel(const RAML::ActionPtr &action)
{
    OC_LOG(DEBUG, TAG, "Creating request model");

    // Validate the action type. Only GET, PUT, POST and DELETE are supported.
    RAML::ActionType actionType = action->getType();
    if (actionType != RAML::ActionType::GET
        && actionType != RAML::ActionType::PUT
        && actionType != RAML::ActionType::POST
        && actionType != RAML::ActionType::DELETE)
    {
        OC_LOG(ERROR, TAG, "Failed to create request model as it is of unknown type!");
        return nullptr;
    }

    // Construct RequestModel
    RequestModelSP requestModel(new RequestModel(getRequestType(actionType)));

    // Get the allowed query parameters of the request
    for (auto & qpEntry : action->getQueryParameters())
    {
        for (auto & value :  (qpEntry.second)->getEnumeration())
        {
            requestModel->addQueryParam(qpEntry.first, value);
        }
    }

    RAML::RequestResponseBodyPtr requestBody = action->getRequestBody("application/json");
    SimulatorResourceModelSP repSchema = createRepSchema(requestBody);
    requestModel->setRepSchema(repSchema);

    // Corresponsing responses
    for (auto  & responseEntry :  action->getResponses())
    {
        std::string codeStr = responseEntry.first;
        int code = boost::lexical_cast<int>(codeStr);
        ResponseModelSP responseModel = createResponseModel(code, responseEntry.second);
        if (nullptr != responseModel)
        {
            requestModel->addResponseModel(code, responseModel);
        }
    }

    return requestModel;
}

ResponseModelSP RequestModelBuilder::createResponseModel(int code,
        const RAML::ResponsePtr &response)
{
    ResponseModelSP responseModel(new ResponseModel(code));
    RAML::RequestResponseBodyPtr responseBody = response->getResponseBody("application/json");
    SimulatorResourceModelSP repSchema = createRepSchema(responseBody);
    responseModel->setRepSchema(repSchema);
    return responseModel;
}

SimulatorResourceModelSP RequestModelBuilder::createRepSchema(const RAML::RequestResponseBodyPtr
        &rep)
{
    if (!rep)
    {
        return nullptr;
    }
    RAML::SchemaPtr schema = rep->getSchema();
    if (!schema)
    {
        return nullptr;
    }

    RAML::JsonSchemaPtr properties = schema->getProperties();
    if (!properties || 0 == properties->getProperties().size())
        return nullptr;

    SimulatorResourceModelSP repSchema = std::make_shared<SimulatorResourceModel>();
    for (auto & propertyEntry : properties->getProperties())
    {
        std::string propName = propertyEntry.second->getName();
        if ("rt" == propName || "resourceType" == propName || "if" == propName
            || "p" == propName || "n" == propName || "id" == propName)
            continue;

        int valueType = propertyEntry.second->getValueType();
        switch (valueType)
        {
            case 0: // Integer
                {
                    // Add the attribute with value
                    repSchema->addAttribute(propertyEntry.second->getName(), propertyEntry.second->getValue<int>());

                    // Set the range
                    int min, max, multipleof;
                    propertyEntry.second->getRange(min, max, multipleof);
                    repSchema->setRange(propertyEntry.second->getName(), min, max);
                }
                break;

            case 1: // Double
                {
                    // Add the attribute with value
                    repSchema->addAttribute(propertyEntry.second->getName(), propertyEntry.second->getValue<double>());

                    std::vector<SimulatorResourceModel::Attribute::ValueVariant> propValues =
                        propertyEntry.second->getAllowedValues();

                    // TODO: Use RAML function once available
                    if (0 < propertyEntry.second->getAllowedValuesSize())
                    {
                        std::vector<double> allowedValues;
                        for (auto & propValue : propValues)
                        {
                            double value = boost::lexical_cast<double> (propValue);
                            allowedValues.push_back(value);
                        }
                        repSchema->setAllowedValues(propertyEntry.second->getName(), allowedValues);
                    }
                }
                break;

            case 2: // Boolean
                {
                    // Add the attribute with value
                    repSchema->addAttribute(propertyEntry.second->getName(), propertyEntry.second->getValue<bool>());
                }
                break;

            case 3: // String
                {
                    // Add the attribute with value
                    repSchema->addAttribute(propertyEntry.second->getName(),
                                            propertyEntry.second->getValue<std::string>());

                    std::vector<SimulatorResourceModel::Attribute::ValueVariant> propValues =
                        propertyEntry.second->getAllowedValues();

                    // TODO: Use RAML function once available
                    if (0 < propertyEntry.second->getAllowedValuesSize())
                    {
                        std::vector<std::string> allowedValues;
                        for (auto & propValue : propValues)
                        {
                            std::string value = boost::lexical_cast<std::string> (propValue);
                            allowedValues.push_back(value);
                        }
                        repSchema->setAllowedValues(propertyEntry.second->getName(), allowedValues);
                    }
                }
                break;
        }
    }

    return repSchema;
}

RequestType RequestModelBuilder::getRequestType(RAML::ActionType actionType)
{
    switch (actionType)
    {
        case RAML::ActionType::GET:
            return RequestType::RQ_TYPE_GET;
        case RAML::ActionType::PUT:
            return RequestType::RQ_TYPE_PUT;
        case RAML::ActionType::POST:
            return RequestType::RQ_TYPE_POST;
        case RAML::ActionType::DELETE:
            return RequestType::RQ_TYPE_DELETE;
    }
}

