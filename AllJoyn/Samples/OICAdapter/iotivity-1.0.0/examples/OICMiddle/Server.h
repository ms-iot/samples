#ifndef SERVER_H
#define SERVER_H

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

#include "OICMiddle.h"

class MiddleServer
{
private:
    std::string m_name;
    bool m_state;
    int m_power;
    std::string m_url;
    OCResourceHandle m_resourceHandle;
    OCRepresentation *m_rep;
    std::function<OCEntityHandlerResult(const std::shared_ptr<OCResourceRequest>)> cb;

public:
    MiddleServer();

    bool init();
    bool createAndRegisterResources(std::vector<std::string> &resourceUrlList,
                       std::vector<std::string> &resourceTypeList,
                       std::vector<std::string> &resourceInterfaceList,
                       std::vector<std::string> &nameList,
                       std::vector<std::string> &powerList,
                       std::vector<std::string> &stateList);
    OCEntityHandlerResult entityHandler(const std::shared_ptr<OCResourceRequest>);

    bool registerResource(std::string & resourceUrl,
                       const std::string &resourceTypeName,
                       const std::string & resourceInterface);
private:
    void printRegisterResourceResult(OCStackResult &result);
};

#endif // SERVER_H
