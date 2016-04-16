/*
 * //******************************************************************
 * //
 * // Copyright 2015 Intel Corporation.
 * //
 * //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * //
 * // Licensed under the Apache License, Version 2.0 (the "License");
 * // you may not use this file except in compliance with the License.
 * // You may obtain a copy of the License at
 * //
 * //      http://www.apache.org/licenses/LICENSE-2.0
 * //
 * // Unless required by applicable law or agreed to in writing, software
 * // distributed under the License is distributed on an "AS IS" BASIS,
 * // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * // See the License for the specific language governing permissions and
 * // limitations under the License.
 * //
 * //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

package org.iotivity.base;

import java.util.EnumSet;
import java.util.List;
import java.util.Map;

/**
 * OcResourceRequest provides APIs to extract details from a request
 */
public class OcResourceRequest {

    private OcResourceRequest(long nativeHandle) {
        this.mNativeHandle = nativeHandle;
    }

    /**
     * Retrieves the type of request for the entity handler function to operate
     *
     * @return request type. This could be 'GET'/'PUT'/'POST'/'DELETE'
     */
    public RequestType getRequestType() {
        return RequestType.get(getRequestTypeNative());
    }

    private native String getRequestTypeNative();

    /**
     * Retrieves the query parameters from the request
     *
     * @return parameters in the request
     */
    public native Map<String, String> getQueryParameters();

    /**
     * Retrieves the request handler flag set. This can be INIT flag and/or REQUEST flag and/or
     * OBSERVE flag.
     * NOTE:
     * INIT indicates that the vendor's entity handler should go and perform
     * initialization operations
     * REQUEST indicates that it is a request of certain type (GET/PUT/POST/DELETE) and entity
     * handler needs to perform corresponding operations
     * OBSERVE indicates that the request is of type Observe and entity handler needs to perform
     * corresponding operations
     *
     * @return Set of handler flags
     */
    public EnumSet<RequestHandlerFlag> getRequestHandlerFlagSet() {
        return RequestHandlerFlag.convertToEnumSet(getRequestHandlerFlagNative());
    }

    private native int getRequestHandlerFlagNative();

    /**
     * Provides the entire resource attribute representation
     *
     * @return OcRepresentation object containing the name value pairs representing
     * the resource's attributes
     */
    public native OcRepresentation getResourceRepresentation();

    /**
     * Object provides observation information
     *
     * @return observation information
     */
    public native ObservationInfo getObservationInfo();

    /**
     * Specifies the resource uri
     *
     * @param resourceUri resource uri
     */
    public native void setResourceUri(String resourceUri);

    /**
     * Gets the resource URI
     *
     * @return resource URI
     */
    public native String getResourceUri();

    /**
     * This API retrieves a list of headerOptions which was sent from a client
     *
     * @return List of header options
     */
    public native List<OcHeaderOption> getHeaderOptions();

    /**
     * This API retrieves the request handle
     *
     * @return request handle
     */
    public native OcRequestHandle getRequestHandle();

    /**
     * This API retrieves the resource handle
     *
     * @return resource handle
     */
    public native OcResourceHandle getResourceHandle();

    @Override
    protected void finalize() throws Throwable {
        super.finalize();

        dispose();
    }

    private native void dispose();

    private long mNativeHandle;
}
