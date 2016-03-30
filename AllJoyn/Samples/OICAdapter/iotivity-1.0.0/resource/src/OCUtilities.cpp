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

#include <OCApi.h>

#include <OCUtilities.h>

#include <boost/algorithm/string.hpp>

#include <sstream>
#include <iterator>
#include <algorithm>

OC::Utilities::QueryParamsKeyVal OC::Utilities::getQueryParams(const std::string& uri)
{
    OC::Utilities::QueryParamsKeyVal qp;
    if(uri.empty())
    {
        return qp;
    }

    std::vector<std::string> queryparams;
    boost::split(queryparams, uri, boost::is_any_of(OC_QUERY_SEPARATOR), boost::token_compress_on);

    for(std::string& it: queryparams)
    {
        auto index = it.find('=');

        if(index == std::string::npos)
        {
            qp[it] = "";
        }
        else
        {
            qp[it.substr(0, index)] = it.substr(index + 1);
        }
    }
        return qp;
    }

namespace OC {

OCStackResult result_guard(const OCStackResult r)
{
 std::ostringstream os;

 switch(r)
 {
    default:
        os << "result_guard(): unhandled exception: " << OCException::reason(r);
        throw OCException(os.str(), r);

    /* Exceptional conditions: */
    case OC_STACK_NO_MEMORY:
    case OC_STACK_COMM_ERROR:
    case OC_STACK_NOTIMPL:
    case OC_STACK_INVALID_URI:
    case OC_STACK_INVALID_QUERY:
    case OC_STACK_INVALID_IP:
    case OC_STACK_INVALID_PORT:
    case OC_STACK_INVALID_CALLBACK:
    case OC_STACK_INVALID_METHOD:
    case OC_STACK_INVALID_PARAM:
    case OC_STACK_INVALID_OBSERVE_PARAM:
        os << "result_guard(): " << r << ": " << OCException::reason(r);
        throw OCException(os.str(), r);

    /* Non-exceptional failures or success: */
    case OC_STACK_OK:
    case OC_STACK_NO_RESOURCE:
    case OC_STACK_RESOURCE_ERROR:
    case OC_STACK_SLOW_RESOURCE:
    case OC_STACK_NO_OBSERVERS:
    case OC_STACK_OBSERVER_NOT_FOUND:
#ifdef WITH_PRESENCE
    case OC_STACK_PRESENCE_STOPPED:
    case OC_STACK_PRESENCE_TIMEOUT:
    case OC_STACK_PRESENCE_DO_NOT_HANDLE:
#endif
    break;
 }

 return r;
}

} // namespace OC

