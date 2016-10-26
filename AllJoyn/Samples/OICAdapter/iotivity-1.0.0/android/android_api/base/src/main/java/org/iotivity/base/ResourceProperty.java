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

/**
 * Resource Properties.
 */
public enum ResourceProperty {
    /**
     * When none of the bits are set, the resource is non-discoverable &
     * non-observable by the client.
     */
    RES_PROP_NONE(0),
    /**
     * When this bit is set, the resource is allowed to be discovered by clients.
     */
    DISCOVERABLE(1 << 0),
    /**
     * When this bit is set, the resource is allowed to be observed by clients.
     */
    OBSERVABLE(1 << 1),
    /**
     * When this bit is set, the resource is initialized, otherwise the resource
     * is 'inactive'. 'inactive' signifies that the resource has been marked for
     * deletion or is already deleted.
     */
    ACTIVE(1 << 2),
    /**
     * When this bit is set, the resource has been marked as 'slow'.
     * 'slow' signifies that responses from this resource can expect delays in
     * processing its requests from clients.
     */
    SLOW(1 << 3),
    /**
     * When this bit is set, the resource is a secure resource.
     */
    SECURE(1 << 4),
    /**
     * When this bit is set, the resource is allowed to be discovered only
     * if discovery request contains an explicit querystring.
     * Ex: GET /oic/res?rt=oic.sec.acl
     */
    EXPLICIT_DISCOVERABLE(1 << 5),;

    private int value;

    private ResourceProperty(int value) {
        this.value = value;
    }

    public int getValue() {
        return this.value;
    }
}
