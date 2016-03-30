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

#ifndef COMMON_INTERNAL_ASSERTUTILS_H
#define COMMON_INTERNAL_ASSERTUTILS_H

#include <cstdint>

#include <memory>

#include <octypes.h>
#include <OCException.h>

#include <RCSException.h>

namespace OIC
{
    namespace Service
    {
        namespace Detail
        {
            struct NotOCStackResult;

            template <typename FUNC, typename ...PARAMS>
            struct ResultType
            {
                typedef decltype(std::declval<FUNC>()(std::declval<PARAMS>()...)) type;
            };

            template< typename A, typename B, typename ENABLER = void >
            struct EnableIfTypeIs;

            template< typename A >
            struct EnableIfTypeIs< A, OCStackResult,
                    typename std::enable_if< std::is_same< A, OCStackResult >::value >::type >
            {
                typedef void type;
            };

            template< typename A >
            struct EnableIfTypeIs< A, NotOCStackResult,
                    typename std::enable_if< !std::is_same< A, OCStackResult >::value >::type >
            {
                typedef A type;
            };

            template< typename T, typename = typename std::enable_if<
                    std::is_class<T>::value && std::is_pointer<T>::value>::type >
            struct EnableIfClassPointer
            {
                typedef void type;
            };

            template< typename T, typename = typename std::enable_if< std::is_class< T >::value > >
            struct EnableIfClass
            {
                typedef void type;
            };
        }

        inline void expectOCStackResult(OCStackResult actual,
                std::initializer_list<OCStackResult> allowed)
        {
            for (auto r : allowed)
            {
                if (actual == r) return;
            }

            throw RCSPlatformException(actual);
        }

        inline void expectOCStackResult(OCStackResult actual, OCStackResult expected)
        {
            if (actual != expected)
            {
                throw RCSPlatformException(actual);
            }
        }

        inline void expectOCStackResultOK(OCStackResult actual)
        {
            expectOCStackResult(actual, OC_STACK_OK);
        }

        template< typename FUNC, typename ...PARAMS >
        typename Detail::EnableIfTypeIs< typename Detail::ResultType< FUNC, PARAMS... >::type,
                OCStackResult >::type
        invokeOCFuncWithResultExpect(std::initializer_list<OCStackResult> allowed,
                FUNC&& fn, PARAMS&& ...params)
        {
            try
            {
                expectOCStackResult(fn(std::forward< PARAMS >(params)...), std::move(allowed));
            }
            catch (const OC::OCException& e)
            {
                throw RCSPlatformException(e.code());
            }
        }


        template< typename FUNC, typename ...PARAMS >
        typename Detail::EnableIfTypeIs< typename Detail::ResultType< FUNC, PARAMS... >::type,
                OCStackResult >::type
        invokeOCFunc(FUNC&& fn, PARAMS&& ...params)
        {
            try
            {
                expectOCStackResultOK(fn(std::forward< PARAMS >(params)...));
            }
            catch (const OC::OCException& e)
            {
                throw RCSPlatformException(e.code());
            }
        }

        template< typename FUNC, typename ...PARAMS >
        typename Detail::EnableIfTypeIs< typename Detail::ResultType< FUNC, PARAMS... >::type,
                        Detail::NotOCStackResult >::type
        invokeOCFunc(FUNC* fn, PARAMS&& ...params)
        {
            try
            {
                return fn(std::forward< PARAMS >(params)...);
            }
            catch (const OC::OCException& e)
            {
                throw RCSPlatformException(e.code());
            }
        }

        template< typename OBJ, typename = typename Detail::EnableIfClassPointer<OBJ>::type,
                typename FUNC, typename ...PARAMS >
        inline auto invokeOC(OBJ&& obj, FUNC&& fn, PARAMS&& ...params) ->
            typename Detail::EnableIfTypeIs<
                decltype((obj->*fn)(std::forward< PARAMS >(params)...)), OCStackResult>::
                type
        {
            try
            {
                expectOCStackResultOK(obj->*fn(std::forward< PARAMS >(params)...));
            }
            catch (const OC::OCException& e)
            {
                throw RCSPlatformException(e.code());
            }
        }

        template< typename OBJ, typename = typename Detail::EnableIfClassPointer<OBJ>::type,
                typename FUNC, typename ...PARAMS >
        inline auto invokeOC(OBJ&& obj, FUNC&& fn, PARAMS&& ...params) ->
                typename Detail::EnableIfTypeIs<
                    decltype((obj->*fn)(std::forward< PARAMS >(params)...)),
                    Detail::NotOCStackResult>::
                    type
        {
            try
            {
                obj->*fn(std::forward< PARAMS >(params)...);
            }
            catch (const OC::OCException& e)
            {
                throw RCSPlatformException(e.code());
            }
        }

        template< typename OBJ, typename = typename Detail::EnableIfClass<OBJ>::type,
                typename FUNC, typename ...PARAMS >
        inline auto invokeOC(const std::shared_ptr< OBJ >& obj, FUNC&& fn, PARAMS&& ...params) ->
                typename Detail::EnableIfTypeIs<
                    decltype((obj.get()->*fn)(std::forward< PARAMS >(params)...)), OCStackResult>::
                    type
        {
            try
            {
                expectOCStackResultOK((obj.get()->*fn)(std::forward< PARAMS >(params)...));
            }
            catch (const OC::OCException& e)
            {
                throw RCSPlatformException(e.code());
            }
        }

        template< typename OBJ, typename = typename Detail::EnableIfClass< OBJ >::type,
                typename FUNC, typename ...PARAMS >
        inline auto invokeOC(const std::shared_ptr<OBJ>& obj, FUNC&& fn, PARAMS&& ...params) ->
            typename Detail::EnableIfTypeIs<
                   decltype((obj.get()->*fn)(std::forward< PARAMS >(params)...)),
                   Detail::NotOCStackResult>::
                   type
        {
            try
            {
                return (obj.get()->*fn)(std::forward< PARAMS >(params)...);
            }
            catch (const OC::OCException& e)
            {
                throw RCSPlatformException(e.code());
            }
        }

    }
}

#endif // COMMON_INTERNAL_ASSERTUTILS_H
