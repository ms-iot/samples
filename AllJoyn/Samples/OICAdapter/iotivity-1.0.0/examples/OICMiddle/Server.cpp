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

#include "Server.h"
#include "OCPlatform.h"
#include "OCApi.h"

namespace PH = std::placeholders;

MiddleServer *serverObject = nullptr;   // to be filled in by object

MiddleServer::MiddleServer()
{
    cb = std::bind(&MiddleServer::entityHandler, this, std::placeholders::_1);
    serverObject = this;
}

bool MiddleServer::init() {
    return true;
}

OCEntityHandlerResult MiddleServer::entityHandler(const std::shared_ptr<OCResourceRequest> request) {
    if (!request) {
        return OC_EH_OK;
    }

    std::string requestType = request->getRequestType();
    int requestFlag = request->getRequestHandlerFlag();
    bool responseNeeded = false;

    if (requestFlag && RequestHandlerFlag::RequestFlag) {
        if (requestType == "PUT") {
            responseNeeded = true;
        } else if (requestType == "GET") {
            responseNeeded = true;
        } else if (requestType == "POST") {     // handle post requests here
        } else if (requestType == "DELETE") {   // handle delete requests here
        }
    }

    if (requestFlag && RequestHandlerFlag::ObserverFlag) {
    }

    if (responseNeeded) {
        auto response = std::make_shared<OC::OCResourceResponse>();
        response->setRequestHandle(request->getRequestHandle());
        response->setResourceHandle(request->getResourceHandle());
        response->setErrorCode(200);
        response->setResponseResult(OC_EH_OK);
        if (OC_STACK_OK != OCPlatform::sendResponse(response)) {
            return OC_EH_ERROR;
        }
    }
    return OC_EH_OK;
}

// for debug purposes - to see if the result of registerResource is valid or not
void MiddleServer::printRegisterResourceResult(OCStackResult &result) {
    switch (result) {
    case OC_STACK_OK:
        cout << "OC_STACK_OK\n";
        break;
    case OC_STACK_INVALID_URI:
        cout << "OC_STACK_INVALID_URI\n";
        break;
    case OC_STACK_INVALID_QUERY:
        cout << "OC_STACK_INVALID_QUERY\n";
        break;
    case OC_STACK_INVALID_IP:
        cout << "OC_STACK_INVALID_IP\n";
        break;
    case OC_STACK_INVALID_PORT:
        cout << "OC_STACK_INVALID_PORT\n";
        break;
    case OC_STACK_INVALID_CALLBACK:
        cout << "OC_STACK_INVALID_CALLBACK\n";
        break;
    case OC_STACK_INVALID_METHOD:
        cout << "OC_STACK_INVALID_METHOD\n";
        break;
    case OC_STACK_NO_MEMORY:
        cout << "OC_STACK_NO_MEMORY\n";
        break;
    case OC_STACK_COMM_ERROR:
        cout << "OC_STACK_COMM_ERROR\n";
        break;
    case OC_STACK_INVALID_PARAM:
        cout << "OC_STACK_INVALID_PARAM\n";
        break;
    case OC_STACK_NOTIMPL:
        cout << "OC_STACK_NOTIMPL\n";
        break;
    case OC_STACK_NO_RESOURCE:
        cout << "OC_STACK_NO_RESOURCE\n";
        break;
    case OC_STACK_RESOURCE_ERROR:
        cout << "OC_STACK_RESOURCE_ERROR\n";
        break;
    case OC_STACK_SLOW_RESOURCE:
        cout << "OC_STACK_SLOW_RESOURCE\n";
        break;
    case OC_STACK_NO_OBSERVERS:
        cout << "OC_STACK_NO_OBSERVERS\n";
        break;
    case OC_STACK_ERROR:
        cout << "OC_STACK_ERROR\n";
        break;
    default:
            cout << "UNKNOWN\n";
        break;
    }
}

bool MiddleServer::registerResource(std::string & resourceUrl, const std::string& resourceTypeName, const std::string& resourceInterface)
{
    OCResourceHandle resourceHandle;
    // OCResourceProperty is defined ocstack.h
    uint8_t resourceProperty = OC_DISCOVERABLE | OC_OBSERVABLE;

    //uncomment to enable debugging

    // This will internally create and register the resource.
    OCStackResult result = OC::OCPlatform::registerResource(
                           resourceHandle, resourceUrl, resourceTypeName,
                           resourceInterface,
                           cb,
                           resourceProperty);
    // enable this to see the result of registerResource
    //printRegisterResourceResult_(result);
    if (result != OC_STACK_OK) {
        return false;
    }
    return true;
}
