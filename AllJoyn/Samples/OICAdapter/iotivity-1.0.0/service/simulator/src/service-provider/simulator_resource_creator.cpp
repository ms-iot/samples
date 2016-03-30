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

#include "simulator_resource_creator.h"
#include "simulator_logger.h"
#include <RamlParser.h>
#include "logger.h"

#define TAG "SIM_RESOURCE_CREATOR"

unsigned int SimulatorResourceCreator::s_id;
SimulatorResourceServerImplSP SimulatorResourceCreator::createResource(
    const std::string &configPath)
{
    RAML::RamlPtr raml;

    try
    {
        std::shared_ptr<RAML::RamlParser> ramlParser = std::make_shared<RAML::RamlParser>(configPath);
        raml = ramlParser->getRamlPtr();
    }
    catch (RAML::RamlException &e)
    {
        OC_LOG_V(ERROR, TAG, "RAML Exception occured! [%s]", e.what());
        throw;
    }

    std::map<std::string, RAML::RamlResourcePtr> ramlResources = raml->getResources();
    RAML::RamlResourcePtr ramlResource;
    if (0 == ramlResources.size() || (ramlResource = ramlResources.begin()->second) == nullptr)
    {
        OC_LOG(ERROR, TAG, "Zero resources detected from RAML!");
        return nullptr;
    }

    if (ramlResource)
    {
        SimulatorResourceServerImplSP simResource(new SimulatorResourceServerImpl());
        simResource->setName(ramlResource->getDisplayName());
        simResource->setURI(ramlResource->getResourceUri());

        // Get the resource representation schema from GET response body
        RAML::ActionPtr action = ramlResource->getAction(RAML::ActionType::GET);
        if (!action)
        {
            OC_LOG(ERROR, TAG, "Failed to create resource representation schema as it does not"
                   "posess the GET request!");
            return nullptr;
        }

        RAML::ResponsePtr getResponse = action->getResponse("200");
        if (!getResponse)
        {
            OC_LOG(ERROR, TAG, "Resource does not provide valid GET response!");
            return nullptr;
        }

        RAML::RequestResponseBodyPtr responseBody = getResponse->getResponseBody("application/json");
        if (responseBody)
        {
            RAML::JsonSchemaPtr resourceProperties = responseBody->getSchema()->getProperties();
            for ( auto & propertyElement : resourceProperties->getProperties())
            {
                if (!propertyElement.second)
                    continue;

                std::string propName = propertyElement.second->getName();
                if ("rt" == propName || "resourceType" == propName)
                {
                    simResource->setResourceType(propertyElement.second->getValueString());
                    continue;
                }
                else if ("if" == propName)
                {
                    simResource->setInterfaceType(propertyElement.second->getValueString());
                    continue;
                }
                else if ("p" == propName || "n" == propName || "id" == propName)
                {
                    continue;
                }

                // Build representation attribute
                SimulatorResourceModel::Attribute attribute(propName);
                switch (propertyElement.second->getValueType())
                {
                    case 0: // Integer
                        attribute.setValue(propertyElement.second->getValue<int>());
                        break;

                    case 1: // Double
                        attribute.setValue(propertyElement.second->getValue<double>());
                        break;

                    case 2: // Boolean
                        attribute.setValue(propertyElement.second->getValue<bool>());
                        break;

                    case 3: // String
                        attribute.setValue(propertyElement.second->getValue<std::string>());
                        break;
                }

                // Set range/supported values set
                int min = 0, max = 0, multipleof = 0;
                propertyElement.second->getRange(min, max, multipleof);
                attribute.setRange(min, max);

                if (propertyElement.second->getAllowedValuesSize() > 0)
                    attribute.setAllowedValues(propertyElement.second->getAllowedValues());

                simResource->addAttribute(attribute);
            }
        }

        simResource->setURI(constructURI(simResource->getURI()));
        return simResource;
    }

    return nullptr;
}

/**
 * This method appends a unique key to the given URI to make the URI unique in simulator.
 * Example: If input is "/a/light", then the output will be "/a/light/simulator/0" for the first resource
 * and "/a/light/simulator/1" for the second resource and so on.
 */
std::string SimulatorResourceCreator::constructURI(const std::string &uri)
{
    std::ostringstream os;
    os << uri;
    if (!uri.empty() && '/' != uri[uri.length() - 1])
        os << '/';
    os << "simulator/" << s_id++;
    return os.str();
}

