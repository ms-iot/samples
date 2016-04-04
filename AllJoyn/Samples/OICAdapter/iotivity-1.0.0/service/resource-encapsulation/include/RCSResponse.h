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


/**
 * @file
 *
 * This file contains the declaration of classes and its members related to RCSResponse
 */
#ifndef SERVERBUILDER_RCSRESPONSE_H
#define SERVERBUILDER_RCSRESPONSE_H

#include <memory>

namespace OIC
{
    namespace Service
    {
        class RCSResourceAttributes;

        class RequestHandler;
        class SetRequestHandler;

        /**
         * This class provides factory methods to create the response for a received get request.
         * The response consists of an error code and result attributes.
         *
         * @see RCSResourceObject
         */
        class RCSGetResponse
        {
        public:
            /**
             * Creates a default RCSGetResponse.
             * The response will have 200 for the errorCode and the attributes of RCSResourceObject
             * will be set as the result attributes.
             *
             */
            static RCSGetResponse defaultAction();

            /**
             * Creates a RCSGetResponse with error code passed.
             * The attributes of the RCSResourceObject will be set as the result attributes.
             *
             * @param errorCode The error code to set in response.
             *
             */
            static RCSGetResponse create(int errorCode);

            /**
             * Creates a RCSGetResponse with custom attributes.
             * This sends the passed attributes as the result attributes
             * instead of the one the RCSResourceObject holds.
             *
             * @param attrs The attributes to set.
             *
             * @see RCSResourceAttributes
             *
             */
            static RCSGetResponse create(const RCSResourceAttributes& attrs);

            /**
             * @override
             */
            static RCSGetResponse create(RCSResourceAttributes&& attrs);

            /**
             * Creates a RCSGetResponse with error code passed.
             * This sends the passed attributes as the result attributes
             * instead of the one the RCSResourceObject holds.
             *
             * @param attrs The attributes to set.
             * @param errorCode The error code for response.
             *
             * @see RCSResourceAttributes
             *
             */
            static RCSGetResponse create(const RCSResourceAttributes& attrs, int errorCode);

            /**
             * @override
             */
            static RCSGetResponse create(RCSResourceAttributes&& attrs, int errorCode);

            //! @cond
            RequestHandler* getHandler() const;
            //! @endcond

        private:
            RCSGetResponse(std::shared_ptr< RequestHandler >&&);

        private:
            std::shared_ptr< RequestHandler > m_handler;
        };

        /**
         * This class provides factory methods to create the response for a received set request.
         * The response consists of an error code and result attributes.
         *
         * AcceptanceMethod provides ways how the request will be handled.
         *
         * @see RCSResourceObject
         */
        class RCSSetResponse
        {
        public:
            /**
             * Options for handling a set request.
             *
             * This overrides SetRequestHandlerPolicy.
             *
             * @see SetRequestHandlerPolicy
             *
             */
            enum class AcceptanceMethod
            {
                /**
                 * Follow SetRequestHandlerPolicy of the RCSResourceObject.
                 */
                DEFAULT,

                /**
                 * Accept the request attributes even if there is an unknown key or mismatched type.
                 */
                ACCEPT,

                /**
                 * Ignore the request attributes.
                 */
                IGNORE
            };

            /**
             * Creates a default RCSSetResponse that has AcceptanceMethod::DEFAULT.
             * The response will have 200 for the errorCode.
             * The attributes of RCSResourceObject will be set as the result attributes.
             *
             */
            static RCSSetResponse defaultAction();

            /**
             * Creates a default RCSSetResponse that has AcceptanceMethod::ACCEPT.
             * The response will have 200 for the errorCode.
             * The attributes of RCSResourceObject will be set as the result attributes.
             *
             */
            static RCSSetResponse accept();

            /**
             * Creates a RCSSetResponse that has AcceptanceMethod::ACCEPT and error code passed.
             * The attributes of the RCSResourceObject will be set as the result attributes.
             *
             * @param errorCode The error code to set in response.
             *
             */
            static RCSSetResponse accept(int errorCode);

            /**
             * Creates a default RCSSetResponse that has AcceptanceMethod::IGNORE.
             * The response will have 200 for the errorCode.
             * The attributes of RCSResourceObject will be set as the result attributes.
             *
             */
            static RCSSetResponse ignore();

            /**
             * Creates a RCSSetResponse that has AcceptanceMethod::IGNORE and error code passed.
             * The attributes of the RCSResourceObject will be set as the result attributes.
             *
             * @param errorCode The error code to set in response.
             *
             */
            static RCSSetResponse ignore(int errorCode);

            /**
             * Creates a RCSSetResponse that has AcceptanceMethod::DEFAULT and error code passed.
             * The attributes of the RCSResourceObject will be set as the result attributes.
             *
             * @param errorCode The error code to set in response.
             *
             */
            static RCSSetResponse create(int errorCode);

            /**
             * Creates a RCSSetResponse that has AcceptanceMethod::DEFAULT with custom attributes.
             * This sends the passed attributes as the result attributes
             * instead of the one the RCSResourceObject holds.
             *
             * @param attrs The attributes to set.
             *
             * @see RCSResourceAttributes
             *
             */
            static RCSSetResponse create(const RCSResourceAttributes& attrs);

            /**
             * @override
             */
            static RCSSetResponse create(RCSResourceAttributes&& attrs);

            /**
             * Creates a RCSSetResponse with error code passed.
             * This sends the passed attributes as the result attributes
             * instead of the one the RCSResourceObject holds.
             *
             * @param attrs The attributes to set.
             * @param errorCode The error code for response.
             *
             * @see RCSResourceAttributes
             *
             */
            static RCSSetResponse create(const RCSResourceAttributes& attrs, int errorCode);

            /**
             * @override
             */
            static RCSSetResponse create(RCSResourceAttributes&& attrs, int errorCode);


            //! @cond
            SetRequestHandler* getHandler() const;
            //! @endcond

            /**
             * Returns the acceptance method.
             *
             */
            AcceptanceMethod getAcceptanceMethod() const;

            /**
             * Sets the acceptance method for the RCSSetResponse.
             *
             * @param method AcceptanceMethod value to set
             *
             * @return The reference to this RCSSetResponse
             *
             * @see AcceptanceMethod
             *
             */
            RCSSetResponse& setAcceptanceMethod(AcceptanceMethod method);

        private:
            RCSSetResponse(std::shared_ptr< SetRequestHandler >&&);
            RCSSetResponse(std::shared_ptr< SetRequestHandler >&&, AcceptanceMethod);

        private:
            AcceptanceMethod m_acceptanceMethod;
            std::shared_ptr< SetRequestHandler > m_handler;
        };
    }
}

#endif // SERVERBUILDER_RCSRESPONSE_H
