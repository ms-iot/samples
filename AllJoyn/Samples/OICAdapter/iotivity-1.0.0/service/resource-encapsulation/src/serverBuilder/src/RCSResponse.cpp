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

#include <RCSResponse.h>

#include <RequestHandler.h>

#include <cassert>

namespace OIC
{
    namespace Service
    {
        RCSGetResponse RCSGetResponse::defaultAction()
        {
            return std::make_shared< RequestHandler >();
        }

        RCSGetResponse RCSGetResponse::create(int errorCode)
        {
            return RCSGetResponse {
                std::make_shared< RequestHandler >( errorCode) };
        }

        RCSGetResponse RCSGetResponse::create(const RCSResourceAttributes& attrs)
        {
            return RCSGetResponse { std::make_shared< RequestHandler >(attrs) };
        }

        RCSGetResponse RCSGetResponse::create(const RCSResourceAttributes& attrs, int errorCode)
        {
            return RCSGetResponse { std::make_shared< RequestHandler >(attrs, errorCode) };
        }

        RCSGetResponse RCSGetResponse::create(RCSResourceAttributes&& result)
        {
            return RCSGetResponse {
                std::make_shared< RequestHandler >(std::move(result)) };
        }

        RCSGetResponse RCSGetResponse::create(RCSResourceAttributes&& attrs, int errorCode)
        {
            return RCSGetResponse { std::make_shared< RequestHandler >(
                std::move(attrs), errorCode) };
        }

        RCSGetResponse::RCSGetResponse(std::shared_ptr< RequestHandler >&& handler) :
                m_handler{ std::move(handler) }
        {
            assert(m_handler);
        }

        RequestHandler* RCSGetResponse::getHandler() const
        {
            return m_handler.get();
        }


        RCSSetResponse RCSSetResponse::defaultAction()
        {
            return std::make_shared< SetRequestHandler >();
        }

        RCSSetResponse RCSSetResponse::accept()
        {
            return defaultAction().setAcceptanceMethod(AcceptanceMethod::ACCEPT);
        }

        RCSSetResponse RCSSetResponse::accept(int errorCode)
        {
            return create(errorCode).setAcceptanceMethod(AcceptanceMethod::ACCEPT);
        }

        RCSSetResponse RCSSetResponse::ignore()
        {
            return defaultAction().setAcceptanceMethod(AcceptanceMethod::IGNORE);
        }

        RCSSetResponse RCSSetResponse::ignore(int errorCode)
        {
            return create(errorCode).setAcceptanceMethod(AcceptanceMethod::IGNORE);
        }

        RCSSetResponse RCSSetResponse::create(int errorCode)
        {
            return std::make_shared< SetRequestHandler >(errorCode);
        }

        RCSSetResponse RCSSetResponse::create(const RCSResourceAttributes& attrs)
        {
            return std::make_shared< SetRequestHandler >(attrs);
        }

        RCSSetResponse RCSSetResponse::create(const RCSResourceAttributes& attrs, int errorCode)
        {
            return std::make_shared< SetRequestHandler >(attrs, errorCode);
        }

        RCSSetResponse RCSSetResponse::create(RCSResourceAttributes&& result)
        {
            return std::make_shared< SetRequestHandler >(std::move(result));
        }

        RCSSetResponse RCSSetResponse::create(RCSResourceAttributes&& attrs, int errorCode)
        {
            return std::make_shared< SetRequestHandler >(std::move(attrs), errorCode);
        }

        RCSSetResponse::RCSSetResponse(std::shared_ptr< SetRequestHandler >&& handler) :
                m_acceptanceMethod { AcceptanceMethod::DEFAULT },
                m_handler{ std::move(handler) }
        {
        }

        RCSSetResponse::RCSSetResponse(std::shared_ptr< SetRequestHandler >&& handler,
                AcceptanceMethod method) :
                m_acceptanceMethod{ method },
                m_handler{ std::move(handler) }
        {
            assert(m_handler);
        }

        SetRequestHandler* RCSSetResponse::getHandler() const
        {
            return m_handler.get();
        }

        auto RCSSetResponse::getAcceptanceMethod() const -> AcceptanceMethod
        {
            return m_acceptanceMethod;
        }

        RCSSetResponse& RCSSetResponse::setAcceptanceMethod(AcceptanceMethod method)
        {
            m_acceptanceMethod = method;
            return *this;
        }
    }
}
