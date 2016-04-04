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

#include "TestBundleActivator.h"

TestBundleActivator *bundle;

TestBundleActivator::TestBundleActivator()
{
    m_pResourceContainer = nullptr;
    m_pTestResource = nullptr;
}

TestBundleActivator::~TestBundleActivator()
{
    m_pResourceContainer = nullptr;
    m_pTestResource = nullptr;
}

void TestBundleActivator::activateBundle(ResourceContainerBundleAPI *resourceContainer,
        std::string bundleId)
{
    std::cout << "TestBundleActivator::activateBundle .. " << std::endl;
    m_pResourceContainer = resourceContainer;
    m_bundleId = bundleId;
}

void TestBundleActivator::deactivateBundle()
{
    std::cout << "TestBundleActivator::deactivateBundle .. " << std::endl;
    m_pResourceContainer = nullptr;
}

void TestBundleActivator::createResource(resourceInfo resourceInfo)
{
    std::cout << "TestBundleActivator::createResource .. " << std::endl;

    m_pTestResource = std::make_shared< TestBundleResource >();

    m_pTestResource->m_bundleId = m_bundleId;
    m_pTestResource->m_uri = resourceInfo.uri;
    m_pTestResource->m_resourceType = resourceInfo.resourceType;

    m_pResourceContainer->registerResource(m_pTestResource);
}

void TestBundleActivator::destroyResource(BundleResource::Ptr pBundleResource)
{
    std::cout << "TestBundleActivator::destroyResource .. " << std::endl;

    m_pResourceContainer->unregisterResource(pBundleResource);
}

extern "C" void test_externalActivateBundle(ResourceContainerBundleAPI *resourceContainer,
        std::string bundleId)
{
    bundle = new TestBundleActivator();
    bundle->activateBundle(resourceContainer, bundleId);
}

extern "C" void test_externalDeactivateBundle()
{
    bundle->deactivateBundle();
    delete bundle;
}

extern "C" void test_externalCreateResource(resourceInfo resourceInfo)
{
    bundle->createResource(resourceInfo);
}

extern "C" void test_externalDestroyResource(BundleResource::Ptr pBundleResource)
{
    bundle->destroyResource(pBundleResource);
}
