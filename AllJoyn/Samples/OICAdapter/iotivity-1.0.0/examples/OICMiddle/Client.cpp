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

#include <map>
#include <stdexcept>

#include "WrapResource.h"
#include "Client.h"

MiddleClient::MiddleClient()
{
    m_findCB = bind(&MiddleClient::foundOCResource, this, placeholders::_1);
}

bool MiddleClient::init()
{
    findResources();
    return true;
}

void MiddleClient::findResources()
{
    m_resourceMap.clear();

    OC::OCPlatform::findResource("", OC_RSRVD_WELL_KNOWN_URI, CT_DEFAULT, m_findCB);
}

void MiddleClient::foundOCResource(shared_ptr<OCResource> resource)
{
    WrapResource *wres;
    string resourceID = formatResourceID(resource);

    m_mutexFoundCB.lock();

    try {
        wres = m_resourceMap.at(resourceID);
    } catch (const std::out_of_range) {
        wres = new WrapResource(resourceID, resource);
        m_resourceMap[resourceID] = wres;
    }

    m_mutexFoundCB.unlock();

    wres->findTypes();
}

/*
 *  I need a unique ID, so I concatenate the host string and resource uri
 *  It's arbitrary and sufficient.
 */
string MiddleClient::formatResourceID(std::shared_ptr<OCResource> resource)
{
    if(!resource)
    {
        throw invalid_argument("Invalid resource object in formatResourceID");
    }

    return resource->sid() + resource->uri();
}

void MiddleClient::addResource(WrapResource *wres)
{
    if(!wres)
    {
        throw invalid_argument("Invalid WrapResource object in addResource");
    }

    string resourceID = wres->getResourceID();
    try {
        m_resourceMap[resourceID];
    } catch (const std::out_of_range) {
        m_resourceMap[resourceID] = wres;
    }
}
