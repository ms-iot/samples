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

#ifndef _INTEL_OCUTILITIES_H_
#define _INTEL_OCUTILITIES_H_

#include <map>
#include <vector>
#include <memory>
#include <utility>
#include <exception>

#include <OCException.h>
#include <StringConstants.h>

namespace OC {
    namespace Utilities {

        typedef std::map<std::string, std::string> QueryParamsKeyVal;

        /*
         * @brief helper function that parses the query parameters component
         * of a URI into a key-value map.  This function expects the uri
         * parameter to contain the query parameters component of a URI
         * (everything after the '?', excluding anything anchors).
         *
         * Note that output will not perform URL decoding
         */
        QueryParamsKeyVal getQueryParams(const std::string& uri);
    }
}

/* The C++11 standard unfortunately forgot to provide make_unique<>! However, if we're
using C++14 or later, we want to take the standard library's implementation: */
namespace OC {
#if defined(__cplusplus) && __cplusplus < 201300

    template<typename T, typename ...XS>
    std::unique_ptr<T> make_unique(XS&& ...xs)
    {
        return std::unique_ptr<T>(new T(std::forward<XS>(xs)...));
    }

#else
    using std::make_unique;
#endif
} // namespace OC

namespace OC {

    /* Examine an OCStackResult, and either forward its value or raise an exception: */
    OCStackResult result_guard(const OCStackResult r);

    /* Check for a nullptr, and throw an exception if we see one; otherwise, return the
    result of the function call: */
    template <typename PtrT, typename FnT, typename ...ParamTs>
    auto nil_guard(PtrT&& p, FnT&& fn, ParamTs&& ...params) -> OCStackResult
    {
        if(nullptr == p)
        {
            throw OCException(OC::Exception::NIL_GUARD_NULL, OC_STACK_INVALID_PARAM);
        }

        // Note that the parameters are being passed by reference to std::bind. This is not an
        // issue, as it is this function's parameters that are being passed by reference.  So,
        // unless the parameters are being passed by reference to here (or to checked_guard),
        // they won't be modified.
        return std::bind(fn, p, std::ref(params)...)();
    }

    /* Check for nullptr and forward the result of an OC function call on success; raise
    an exception on failure or exceptional result: */
    template <typename PtrT, typename FnT, typename ...ParamTs>
    auto checked_guard(PtrT&& p, FnT&& fn, ParamTs&& ...params) -> OCStackResult
    {
        return result_guard(nil_guard(p, fn, std::forward<ParamTs>(params)...));
    }

} // namespace OC

namespace OC
{
    template<typename T, typename = void>
    struct is_vector
    {
        constexpr static bool value = false;
    };

    template<typename T>
    struct is_vector<T,
        typename std::enable_if<
            std::is_same<T, std::vector<typename T::value_type, typename T::allocator_type>>::value
        >::type
    >
    {
        constexpr static bool value = true;
    };
} // namespace OC

#endif

