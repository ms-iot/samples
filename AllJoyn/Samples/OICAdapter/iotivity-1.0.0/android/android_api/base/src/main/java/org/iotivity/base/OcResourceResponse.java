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

import java.util.List;

/**
 * OcResourceResponse provides APIs to set the response details
 */
public class OcResourceResponse {

    static {
        System.loadLibrary("oc");
        System.loadLibrary("ocstack-jni");
    }

    public OcResourceResponse() {
        super();

        create();
    }

    private OcResourceResponse(long nativeHandle) {
        this.mNativeHandle = nativeHandle;
    }

    /**
     * This API sets the error code for this response
     *
     * @param eCode error code to set
     */
    public native void setErrorCode(int eCode);

    /**
     * Gets new resource uri
     *
     * @return new resource uri
     */
    public native String getNewResourceUri();

    /**
     * Sets new resource uri
     *
     * @param newResourceUri new resource uri
     */
    public native void setNewResourceUri(String newResourceUri);

    /**
     * This API allows to set headerOptions in the response
     *
     * @param headerOptionList List of HeaderOption entries
     */
    public void setHeaderOptions(List<OcHeaderOption> headerOptionList) {
        this.setHeaderOptions(headerOptionList.toArray(
                        new OcHeaderOption[headerOptionList.size()])
        );
    }

    private native void setHeaderOptions(OcHeaderOption[] headerOptionList);

    /**
     * This API allows to set request handle
     *
     * @param ocRequestHandle request handle
     */
    public native void setRequestHandle(OcRequestHandle ocRequestHandle);

    /**
     * This API allows to set the resource handle
     *
     * @param ocResourceHandle resource handle
     */
    public native void setResourceHandle(OcResourceHandle ocResourceHandle);

    /**
     * This API allows to set the EntityHandler response result
     *
     * @param responseResult OcEntityHandlerResult type to set the result value
     */
    public void setResponseResult(EntityHandlerResult responseResult) {
        this.setResponseResult(responseResult.getValue());
    }

    private native void setResponseResult(int responseResult);

    /**
     * API to set the entire resource attribute representation
     *
     * @param ocRepresentation the name value pairs representing the resource's attributes
     * @param interfaceStr     specifies the interface
     */
    public native void setResourceRepresentation(OcRepresentation ocRepresentation,
                                                 String interfaceStr);

    /**
     * API to set the entire resource attribute representation
     *
     * @param representation object containing the name value pairs representing the
     *                       resource's attributes
     */
    public void setResourceRepresentation(OcRepresentation representation) {
        this.setResourceRepresentation1(representation);
    }

    private native void setResourceRepresentation1(OcRepresentation representation);

    @Override
    protected void finalize() throws Throwable {
        super.finalize();

        dispose();
    }

    private native void create();

    private native void dispose();

    private long mNativeHandle;
}
