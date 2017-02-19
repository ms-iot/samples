#ifndef CLIENT_H
#define CLIENT_H

//******************************************************************
//
// Copyright 2014 Intel Corporation.
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

#include <mutex>

#include "OICMiddle.h"

typedef map<string, WrapResource *> resourcemap_t;
typedef pair<string, WrapResource *> resourcemappair_t;

class MiddleClient
{
public:
    MiddleClient();

    bool init();
    void findResources();

    friend class LineInput;
    friend class HueResource;
    friend class HueResources;
    friend class RestInput;

protected:
    mutex m_mutexFoundCB;
    map<string, WrapResource *> m_resourceMap;
    HueResources *m_hueResources;
    std::function<void(std::shared_ptr<OCResource> resource)> m_findCB;

    void foundOCResource(shared_ptr<OCResource> resource);
    string formatResourceID(std::shared_ptr<OCResource> resource);
    void findHueResources();
    void addResource(WrapResource *wres);
};


#endif // CLIENT_H

