/******************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

package org.iotivity.service.server;

import org.iotivity.service.RcsResourceAttributes;

/**
 * This class provides methods to create the response for a received set
 * request.
 *
 * @see RcsResourceObject
 * @see RcsSetResponse
 */
public final class RcsSetResponse extends RcsResponse {
    /**
     * Options for handling a set request.
     *
     * This overrides {@link RcsResourceObject.SetRequestHandlerPolicy}.
     *
     */
    public enum AcceptanceMethod {
        /**
         * Follow {@link RcsResourceObject.SetRequestHandlerPolicy}.
         */
        DEFAULT,

        /**
         * Accept the request attributes even if there is an unknown key or
         * mismatched type.
         */
        ACCEPT,

        /**
         * Ignore the request attributes.
         */
        IGNORE
    };

    private AcceptanceMethod mAcceptanceMethod = AcceptanceMethod.DEFAULT;

    /**
     * Creates a default RcsSetResponse with {@link AcceptanceMethod#DEFAULT}.
     * The response will have {@link #DEFAULT_ERROR_CODE} for the errorCode. The
     * attributes of {@link RcsResourceObject} will be set as the result
     * attributes.
     *
     */
    public static RcsSetResponse defaultAction() {
        return new RcsSetResponse();
    }

    /**
     * Creates a default RcsSetResponse with {@link AcceptanceMethod#ACCEPT}
     * The response will have {@link #DEFAULT_ERROR_CODE} for the errorCode. The
     * attributes of {@link RcsResourceObject} will be set as the result
     * attributes.
     *
     */
    public static RcsSetResponse accept() {
        return new RcsSetResponse()
                .setAcceptanceMethod(AcceptanceMethod.ACCEPT);
    }

    /**
     * Creates a RcsSetResponse with {@link AcceptanceMethod#ACCEPT} and error
     * code passed.
     * The attributes of the {@link RcsResourceObject} will be set as the result
     * attributes.
     *
     * @param errorCode
     *            error code to be set in response
     *
     */
    public static RcsSetResponse accept(int errorCode) {
        return new RcsSetResponse(errorCode)
                .setAcceptanceMethod(AcceptanceMethod.ACCEPT);
    }

    /**
     * Creates a default RcsSetResponse with {@link AcceptanceMethod#IGNORE}.
     * The response will have {@link #DEFAULT_ERROR_CODE} for the errorCode. The
     * attributes of {@link RcsResourceObject} will be set as the result
     * attributes.
     *
     */
    public static RcsSetResponse ignore() {
        return new RcsSetResponse()
                .setAcceptanceMethod(AcceptanceMethod.IGNORE);
    }

    /**
     * Creates a RcsSetResponse with {@link AcceptanceMethod#IGNORE} and error
     * code passed. The attributes of the {@link RcsResourceObject} will be set
     * as the result attributes.
     *
     * @param errorCode
     *            error code to be set in response
     *
     */
    public static RcsSetResponse ignore(int errorCode) {
        return new RcsSetResponse(errorCode)
                .setAcceptanceMethod(AcceptanceMethod.IGNORE);
    }

    /**
     * Creates a RcsSetResponse with error code passed and
     * {@link AcceptanceMethod#DEFAULT}. The attributes of the
     * {@link RcsResourceObject} will be set as the result attributes.
     *
     * @param errorCode
     *            error code to be set in response
     *
     */
    public static RcsSetResponse create(int errorCode) {
        return new RcsSetResponse(errorCode);
    }

    /**
     * Creates a RcsSetResponse with custom attributes and
     * {@link AcceptanceMethod#DEFAULT}. This sends the passed attributes as the
     * result attributes instead of one the {@link RcsResourceObject} holds.
     *
     * @param attributes
     *            attributes to be sent as the result
     *
     */
    public static RcsSetResponse create(RcsResourceAttributes attributes) {
        return new RcsSetResponse(attributes);
    }

    /**
     * Creates a RcsSetResponse with error code passed and
     * {@link AcceptanceMethod#DEFAULT}. This sends the passed attributes as the
     * result attributes instead of one the {@link RcsResourceObject} holds.
     *
     * @param attributes
     *            attributes to be sent as the result
     * @param errorCode
     *            error code for response
     *
     */
    public static RcsSetResponse create(RcsResourceAttributes attributes,
            int errorCode) {
        return new RcsSetResponse(attributes, errorCode);
    }

    /**
     * Returns the acceptance method.
     *
     */
    public AcceptanceMethod getAcceptanceMethod() {
        return mAcceptanceMethod;
    }

    /**
     * Sets the acceptance method.
     *
     * @param method
     *            method to be set
     *
     * @return The reference to this RcsSetResponse
     *
     */
    public RcsSetResponse setAcceptanceMethod(AcceptanceMethod method) {
        mAcceptanceMethod = method;
        return this;
    }

    private RcsSetResponse() {
        super();
    }

    private RcsSetResponse(int errorCode) {
        super(errorCode);
    }

    private RcsSetResponse(RcsResourceAttributes attrs) {
        super(attrs);
    }

    private RcsSetResponse(RcsResourceAttributes attrs, int errorCode) {
        super(attrs, errorCode);
    }

}
