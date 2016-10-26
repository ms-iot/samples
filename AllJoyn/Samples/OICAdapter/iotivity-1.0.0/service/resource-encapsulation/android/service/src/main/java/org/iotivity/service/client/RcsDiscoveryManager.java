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

package org.iotivity.service.client;

import org.iotivity.service.RcsException;
import org.iotivity.service.RcsObject;
import org.iotivity.service.RcsPlatformException;

/**
 * This class contains the resource discovery methods.
 *
 * @see RcsRemoteResourceObject
 */
public final class RcsDiscoveryManager {

    static {
        System.loadLibrary("gnustl_shared");
        System.loadLibrary("oc_logger");
        System.loadLibrary("connectivity_abstraction");
        System.loadLibrary("ca-interface");
        System.loadLibrary("octbstack");
        System.loadLibrary("oc");
        System.loadLibrary("rcs_client");
        System.loadLibrary("rcs_server");
        System.loadLibrary("rcs_common");
        System.loadLibrary("rcs_jni");
    }

    /**
     * This represents a task for discovery.
     *
     * The task must be canceled if no longer needed.
     *
     */
    public static class DiscoveryTask extends RcsObject {
        private DiscoveryTask() {
        }

        public void cancel() {
            dispose();
        }
    }

    /**
     * Interface definition for a callback to be invoked when a resource is
     * discovered.
     *
     */
    public interface OnResourceDiscoveredListener {

        /**
         * Called when a resource is discovered.
         *
         * @param rcsRemoteResourceObject
         *            a discovered remote resource
         *
         */
        void onResourceDiscovered(
                RcsRemoteResourceObject rcsRemoteResourceObject);

    }

    private static final RcsDiscoveryManager sInstance = new RcsDiscoveryManager();

    private native DiscoveryTask nativeDiscoverResource(String address,
            String relativeUri, String resourceType,
            OnResourceDiscoveredListener listener);

    private RcsDiscoveryManager() {
    }

    public static RcsDiscoveryManager getInstance() {
        return sInstance;
    }

    /**
     * Requests discovery for the resource of interest, regardless of uri and
     * resource type
     *
     * @param address
     *            the target address
     * @param listener
     *            the listener to be invoked when a resource is discovered
     *
     * @return a task object indicating this request.
     *
     * @throws RcsPlatformException
     *             if the operation failed.
     * @throws NullPointerException
     *             if address or listener is null.
     *
     */
    public DiscoveryTask discoverResource(RcsAddress address,
            OnResourceDiscoveredListener listener) throws RcsException {
        return discoverResourceByType(address, null, null, listener);
    }

    /**
     * Requests discovery for the resource of interest, regardless of resource
     * type.
     *
     * @param address
     *            the target address
     * @param uri
     *            the relative uri of resource to be searched
     * @param listener
     *            the listener to be invoked when a resource is discovered
     *
     * @return a task object indicating this request.
     *
     * @throws RcsPlatformException
     *             if the operation failed.
     * @throws NullPointerException
     *             if address or listener is null.
     *
     */
    public DiscoveryTask discoverResource(RcsAddress address, String uri,
            OnResourceDiscoveredListener listener) throws RcsException {
        return discoverResourceByType(address, uri, null, listener);
    }

    /**
     * Requests discovery for the resource of interest by resource type.
     *
     * @param address
     *            the target address
     * @param resourceType
     *            the resource type
     * @param listener
     *            the listener to be invoked when a resource is discovered
     *
     * @return a task object indicating this request.
     *
     * @throws RcsPlatformException
     *             if the operation failed.
     * @throws NullPointerException
     *             if address or listener is null.
     *
     *
     */
    public DiscoveryTask discoverResourceByType(RcsAddress address,
            String resourceType, OnResourceDiscoveredListener listener)
                    throws RcsException {
        return discoverResourceByType(address, null, resourceType, listener);
    }

    /**
     * Requests discovery for the resource of interest by resource type with
     * provided relative uri.
     *
     * @param address
     *            the target address
     * @param uri
     *            the relative uri of resource to be searched
     * @param resourceType
     *            the resource type
     * @param listener
     *            the listener to be invoked when a resource is discovered
     *
     * @return a task object indicating this request.
     *
     * @throws RcsPlatformException
     *             if the operation failed.
     * @throws NullPointerException
     *             if address or listener is null.
     *
     *
     */
    public DiscoveryTask discoverResourceByType(RcsAddress address, String uri,
            String resourceType, OnResourceDiscoveredListener listener)
                    throws RcsException {
        if (listener == null) {
            throw new NullPointerException("listener is null.");
        }
        if (address == null) {
            throw new NullPointerException("address is null.");
        }

        return nativeDiscoverResource(address.getAddress(), uri, resourceType,
                listener);
    }

}
