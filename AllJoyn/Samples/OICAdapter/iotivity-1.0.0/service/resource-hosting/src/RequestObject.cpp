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

#include "RequestObject.h"

namespace OIC
{
namespace Service
{

RequestObject::RequestObject() : pSetRequestCB(nullptr){ }
RequestObject::RequestObject(SetRequestCallback cb) : pSetRequestCB(cb){ };

RequestObject::~RequestObject()
{
    pSetRequestCB = {};
}

void RequestObject::invokeRequest(RemoteObjectPtr remoteObject, RequestMethod method,
        RCSResourceAttributes & resourceAttibutes)
{
    try
    {
        switch (method)
        {
        case RequestMethod::Setter:
        {
            if(pSetRequestCB == nullptr)
            {
                remoteObject->setRemoteAttributes(resourceAttibutes,
                        std::bind(&RequestObject::setRequestCB, this,
                                std::placeholders::_1, resourceAttibutes));
            }
            else
            {
                remoteObject->setRemoteAttributes(resourceAttibutes,
                        std::bind(pSetRequestCB,
                                std::placeholders::_1, resourceAttibutes));
            }
        }
            break;
        case RequestMethod::Getter:
        case RequestMethod::Delete:
        default:
            // unknown type of method.
            break;
        }
    }catch(...)
    {
        throw;
    }
}

void RequestObject::setRequestCB(const RCSResourceAttributes & returnedAttributes,
        RCSResourceAttributes & putAttibutes)
{
    if(putAttibutes != returnedAttributes)
    {
        // TODO fail set attributes
    }
}

} /* namespace Service */
} /* namespace OIC */
