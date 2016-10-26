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

#include <OCResourceRequest.h>

#include <vector>
#include <map>
#include "ocpayload.h"

using namespace OC;

void OCResourceRequest::setPayload(OCPayload* payload)
{
    MessageContainer info;

    if(payload == nullptr)
    {
        return;
    }
    if(payload->type != PAYLOAD_TYPE_REPRESENTATION)
    {
        throw std::logic_error("Wrong payload type");
        return;
    }

    info.setPayload(payload);

    const std::vector<OCRepresentation>& reps = info.representations();
    if(reps.size() >0)
    {
        std::vector<OCRepresentation>::const_iterator itr = reps.begin();
        std::vector<OCRepresentation>::const_iterator back = reps.end();
        m_representation = *itr;
        ++itr;

        for(;itr != back; ++itr)
        {
            m_representation.addChild(*itr);
        }
    }
    else
    {
        oclog() << "setPayload Error: "<<OC::Exception::INVALID_REPRESENTATION<< std::flush;
    }
}
