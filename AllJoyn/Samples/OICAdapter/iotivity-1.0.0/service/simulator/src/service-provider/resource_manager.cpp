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

#include "resource_manager.h"
#include "simulator_logger.h"
#include "logger.h"

#define TAG "RESOURCE_MANAGER"

ResourceManager *ResourceManager::getInstance()
{
    static ResourceManager s_instance;
    return &s_instance;
}

SimulatorResourceServerSP ResourceManager::createResource(const std::string &configPath,
        SimulatorResourceServer::ResourceModelChangedCB callback)
{
    OC_LOG_V(INFO, "Create resource request : config=%s", configPath.c_str());

    // Input validation
    if (configPath.empty())
    {
        OC_LOG(ERROR, TAG, "Invalid config file path!");
        throw InvalidArgsException(SIMULATOR_INVALID_PARAM, "Invalid RAML file path!");
    }

    if (!callback)
    {
        OC_LOG(ERROR, TAG, "Invalid callback!");
        throw InvalidArgsException(SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
    }

    return buildResource(configPath, callback);
}

std::vector<SimulatorResourceServerSP> ResourceManager::createResource(
    const std::string &configPath, unsigned short count,
    SimulatorResourceServer::ResourceModelChangedCB callback)
{
    OC_LOG_V(INFO, "Create multiple resource request : config=%s, count=%d", configPath.c_str(),
             count);

    // Input validation
    if (configPath.empty())
    {
        OC_LOG(ERROR, TAG, "Invalid config file path!");
        throw InvalidArgsException(SIMULATOR_INVALID_PARAM, "Invalid RAML file path!");
    }

    if (0 == count)
    {
        OC_LOG(ERROR, TAG, "Invalid count value!");
        throw InvalidArgsException(SIMULATOR_INVALID_CALLBACK, "Invalid count value!");
    }

    if (!callback)
    {
        OC_LOG(ERROR, TAG, "Invalid callback!");
        throw InvalidArgsException(SIMULATOR_INVALID_CALLBACK, "Invalid callback!");
    }

    std::vector<SimulatorResourceServerSP> resourceList;

    // Create resources
    for (unsigned short i = 0; i < count; i++)
    {
        OC_LOG_V(INFO, TAG, "Creating resource [%d]", i + 1);
        SIM_LOG(ILogger::INFO, "Creating resource [" << i + 1 << "]");

        SimulatorResourceServerSP resource = buildResource(configPath, callback);
        if (!resource)
        {
            break;
        }

        resourceList.push_back(resource);
    }

    SIM_LOG(ILogger::INFO, "[" << resourceList.size() << " out of " << count <<
            "] resource(s) created successfully.");

    return resourceList;
}

std::vector<SimulatorResourceServerSP> ResourceManager::getResources(
    const std::string &resourceType)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);

    std::vector<SimulatorResourceServerSP> resourceList;
    for (auto resourceTableEntry : m_resources)
    {
        if (!resourceType.empty() && resourceType.compare(resourceTableEntry.first))
            continue;

        for (auto resourceEntry : resourceTableEntry.second)
        {
            resourceList.push_back(resourceEntry.second);
        }
    }

    return resourceList;
}

void ResourceManager::deleteResource(const SimulatorResourceServerSP &resource)
{
    if (!resource)
    {
        OC_LOG(ERROR, TAG, "Invalid resource object!");
        throw InvalidArgsException(SIMULATOR_INVALID_PARAM, "Invalid resource object!");
    }

    std::lock_guard<std::recursive_mutex> lock(m_lock);
    auto resourceTableEntry = m_resources.find(resource->getResourceType());
    if (m_resources.end() != resourceTableEntry)
    {
        auto resourceEntry = resourceTableEntry->second.find(resource->getURI());
        if (resourceTableEntry->second.end() != resourceEntry)
        {
            SimulatorResourceServerImplSP resourceImpl =
                std::dynamic_pointer_cast<SimulatorResourceServerImpl>(resource);
            resourceImpl->stop();
            resourceTableEntry->second.erase(resourceEntry);
            SIM_LOG(ILogger::INFO, "Resource (" << resource->getURI() <<
                    ") deleted successfully.");
        }
    }
}

void ResourceManager::deleteResources(const std::string &resourceType)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    for (auto & resourceTableEntry : m_resources)
    {
        if (!resourceType.empty() && resourceType.compare(resourceTableEntry.first))
            continue;

        for (auto & resourceEntry : resourceTableEntry.second)
        {
            SimulatorResourceServerSP resource = resourceEntry.second;
            SimulatorResourceServerImplSP resourceImpl =
                std::dynamic_pointer_cast<SimulatorResourceServerImpl>(resource);
            resourceImpl->stop();
            SIM_LOG(ILogger::INFO, "Resource (" << resource->getURI() <<
                    ") deleted successfully.");
        }

        // Erase the entry for resource type from resources list
        m_resources.erase(resourceTableEntry.first);
    }
}

/**
 * This method does not validate the input given, thus Caller of this method must validate
 * the inputs before invoking this private method.
 */
SimulatorResourceServerSP ResourceManager::buildResource(const std::string &configPath,
        SimulatorResourceServer::ResourceModelChangedCB callback)
{
    // Create resource based on the RAML file.
    SimulatorResourceServerImplSP resourceImpl = m_resourceCreator.createResource(configPath);
    if (!resourceImpl)
    {
        OC_LOG(ERROR, TAG, "Failed to create resource!");
        throw SimulatorException(SIMULATOR_ERROR, "Failed to create resource!");
    }

    resourceImpl->setModelChangeCallback(callback);
    resourceImpl->start();

    // Add the resource to resource list table
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    SimulatorResourceServerSP resource =
        std::dynamic_pointer_cast<SimulatorResourceServer>(resourceImpl);
    m_resources[resourceImpl->getResourceType()].insert(
        std::pair<std::string, SimulatorResourceServerSP>(resourceImpl->getURI(), resourceImpl));

    SIM_LOG(ILogger::INFO, "Created an OIC resource of type [" <<
            resourceImpl->getResourceType() << "]");
    return resourceImpl;
}

/**
 * This method appends a unique key to the given URI to make the URI unique in simulator.
 * Example: If input is "/a/light", then the output will be "/a/light/simulator/0" for the first resource
 * and "/a/light/simulator/1" for the second resource and so on.
 */
std::string ResourceManager::constructURI(const std::string &uri)
{
    std::ostringstream os;
    os << uri;
    if (!uri.empty() && '/' != uri[uri.length() - 1])
        os << '/';
    os << "simulator/" << m_id++;
    return os.str();
}

