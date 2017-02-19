//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
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

#include "HueSampleBundleActivator.h"
#include "HueLight.h"

#include <algorithm>
#include <vector>

using namespace OIC::Service;

HueSampleBundleActivator *bundle;

HueSampleBundleActivator::HueSampleBundleActivator()
{
    m_pResourceContainer = nullptr;
    m_connector = nullptr;
}

HueSampleBundleActivator::~HueSampleBundleActivator()
{
    m_pResourceContainer = nullptr;
    m_connector = nullptr;
}

void HueSampleBundleActivator::activateBundle(ResourceContainerBundleAPI *resourceContainer,
        std::string bundleId)
{

    m_pResourceContainer = resourceContainer;
    m_bundleId = bundleId;
    m_connector = new HueConnector();

    vector< resourceInfo > resourceConfig;

    resourceContainer->getResourceConfiguration(m_bundleId, &resourceConfig);

    for (vector< resourceInfo >::iterator itor = resourceConfig.begin();
         itor != resourceConfig.end(); itor++)
    {
        createResource(*itor);
    }
}

void HueSampleBundleActivator::deactivateBundle()
{
    std::cout << "HueSampleBundle::deactivateBundle called" << std::endl;

    std::vector< BundleResource::Ptr >::iterator itor;
    for (itor = m_vecResources.begin(); itor != m_vecResources.end();)
    {
        destroyResource(*itor);
    }

    delete m_connector;
}

void HueSampleBundleActivator::createResource(resourceInfo resourceInfo)
{

    if (resourceInfo.resourceType == "oic.r.light")
    {
        static int lightCount = 1;
        BundleResource::Ptr hueLight = std::make_shared< HueLight >(m_connector, resourceInfo.address);
        resourceInfo.uri = "/hue/light/" + std::to_string(lightCount++);
        hueLight->m_bundleId = m_bundleId;
        hueLight->m_uri = resourceInfo.uri;
        hueLight->m_resourceType = resourceInfo.resourceType;
        hueLight->m_name = resourceInfo.name;

        m_pResourceContainer->registerResource(hueLight);
        m_vecResources.push_back(hueLight);
    }
}

void HueSampleBundleActivator::destroyResource(BundleResource::Ptr pBundleResource)
{
    std::cout << "HueSampleBundle::destroyResource called" << pBundleResource->m_uri << std::endl;

    std::vector< BundleResource::Ptr >::iterator itor;

    itor = std::find(m_vecResources.begin(), m_vecResources.end(), pBundleResource);

    if (itor != m_vecResources.end())
    {
        m_pResourceContainer->unregisterResource(pBundleResource);
        m_vecResources.erase(itor);
    }
}

extern "C" void huesample_externalActivateBundle(ResourceContainerBundleAPI *resourceContainer,
        std::string bundleId)
{
    bundle = new HueSampleBundleActivator();
    bundle->activateBundle(resourceContainer, bundleId);
}

extern "C" void huesample_externalDeactivateBundle()
{
    bundle->deactivateBundle();
    delete bundle;
}

extern "C" void huesample_externalCreateResource(resourceInfo resourceInfo)
{
    bundle->createResource(resourceInfo);
}

extern "C" void huesample_externalDestroyResource(BundleResource::Ptr pBundleResource)
{
    bundle->destroyResource(pBundleResource);
}
