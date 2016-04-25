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

import org.iotivity.service.RcsDestroyedObjectException;
import org.iotivity.service.RcsException;
import org.iotivity.service.RcsIllegalStateException;
import org.iotivity.service.RcsObject;
import org.iotivity.service.RcsPlatformException;
import org.iotivity.service.RcsResourceAttributes;
import org.iotivity.service.RcsValue;

/**
 *
 * This represents a remote resource and provides simple ways to interact with
 * it.
 * Basically this is a client of a remote resource that runs on other device.
 *
 * The class supports features to help get information of a remote resource
 * such as monitoring and caching.
 *
 * @see RcsDiscoveryManager
 *
 */
public final class RcsRemoteResourceObject extends RcsObject {

    private native boolean nativeIsMonitoring();

    private native boolean nativeIsCaching();

    private native boolean nativeIsObservable();

    private native void nativeStartMonitoring(OnStateChangedListener listener);

    private native void nativeStopMonitoring();

    private native ResourceState nativeGetState();

    private native void nativeStartCaching(OnCacheUpdatedListener listener);

    private native void nativeStopCaching();

    private native CacheState nativeGetCacheState();

    private native boolean nativeIsCachedAvailable();

    private native RcsResourceAttributes nativeGetCachedAttributes();

    private native void nativeGetRemoteAttributes(
            OnRemoteAttributesReceivedListener listener);

    private native void nativeSetRemoteAttributes(
            RcsResourceAttributes attributes,
            OnRemoteAttributesReceivedListener listener);

    private native String nativeGetUri();

    private native String nativeGetAddress();

    private native String[] nativeGetTypes();

    private native String[] nativeGetInterfaces();

    private RcsRemoteResourceObject() {
    }

    /**
     * This represents states of monitoring.
     *
     * @see #startMonitoring(OnStateChangedListener)
     * @see #getState()
     * @see OnStateChangedListener
     *
     */
    public enum ResourceState {
        /** Monitoring is not started. */
        NONE,

        /**
         * Monitoring is started and checking state is in progress.
         * This is the default state after startMonitoring.
         */
        REQUESTED,

        /** The resource is alive. */
        ALIVE,

        /** Failed to reach the resource. */
        LOST_SIGNAL,

        /** The resource is deleted. */
        DESTROYED
    }

    /**
     * This represents states of caching.
     *
     * @see #startCaching()
     * @see #getCacheState()
     */
    public enum CacheState {
        /** Caching is not started. */
        NONE,

        /**
         * Caching is started, but the data is not ready yet. This is
         * the default state after startCaching.
         */
        UNREADY,

        /** The data is ready. */
        READY,

        /** Failed to reach the resource. */
        LOST_SIGNAL
    }

    /**
     * Interface definition for a callback to be invoked when the cache is
     * updated.
     *
     * @see #startCaching(OnCacheUpdatedListener)
     */
    public interface OnCacheUpdatedListener {

        /**
         * Called when the cache is updated.
         *
         * @param attributes
         *            the updated attributes
         *
         */
        public void onCacheUpdated(RcsResourceAttributes attributes);

    }

    /**
     * Interface definition for a callback to be invoked when the response of
     * getRemoteAttributes and setRemoteAttributes is received.
     *
     * @see #getRemoteAttributes(OnRemoteAttributesReceivedListener)
     * @see #setRemoteAttributes(RcsResourceAttributes,
     *      OnRemoteAttributesReceivedListener)
     */
    public interface OnRemoteAttributesReceivedListener {

        /**
         * Called when a response for the getRemoteAttributes request or
         * setRemoteAttributes request is received.
         *
         * @param attributes
         *            the resource attributes received from the remote resource
         *
         */
        public void onAttributesReceived(RcsResourceAttributes attributes,
                int errorCode);

    }

    /**
     * Interface definition for a callback to be invoked when the monitoring
     * state is changed.
     *
     * @see #startMonitoring(OnStateChangedListener)
     */
    public interface OnStateChangedListener {

        /**
         * Called when the monitoring state is changed.
         *
         * @param resourceState
         *            updated state
         *
         */
        public void onStateChanged(ResourceState resourceState);
    }

    private void assertAlive() throws RcsException {
        if (!hasHandle()) {
            throw new RcsDestroyedObjectException(
                    "The object is already destroyed!");
        }
    }

    /**
     * Returns whether monitoring is enabled.
     *
     * @return true if monitoring the resource.
     *
     * @throws RcsDestroyedObjectException
     *             if the object is already destroyed
     *
     * @see #startMonitoring(OnStateChangedListener)
     */
    public boolean isMonitoring() throws RcsException {
        assertAlive();
        return nativeIsMonitoring();
    }

    /**
     * Returns whether caching is enabled.
     *
     * @return true if caching the resource.
     *
     * @throws RcsDestroyedObjectException
     *             if the object is already destroyed
     *
     * @see #startCaching()
     * @see #startCaching(OnCacheUpdatedListener)
     *
     */
    public boolean isCaching() throws RcsException {
        assertAlive();
        return nativeIsCaching();
    }

    /**
     * Returns whether resource is observable.
     *
     * @return true if resource is observable.
     *
     * @throws RcsDestroyedObjectException
     *             if the object is already destroyed
     *
     * @see org.iotivity.service.server.RcsResourceObject.Builder#setObservable(boolean)
     */
    public boolean isObservable() throws RcsException {
        assertAlive();
        return nativeIsObservable();
    }

    /**
     * Starts monitoring the resource.
     * <p>
     * Monitoring provides a feature to check the presence of a resource, even
     * when the server is not announcing Presence using startPresnece.
     *
     * @param listener
     *            the listener to receive new state.
     *
     * @throws NullPointerException
     *             if listener is null
     * @throws RcsIllegalStateException
     *             if monitoring is already started
     * @throws RcsDestroyedObjectException
     *             if the object is already destroyed
     *
     * @see ResourceState
     * @see #isMonitoring()
     * @see #stopMonitoring()
     */
    public void startMonitoring(OnStateChangedListener listener)
            throws RcsException {
        assertAlive();
        if (listener == null) {
            throw new NullPointerException("listener is null.");
        }

        nativeStartMonitoring(listener);
    }

    /**
     * Stops monitoring the resource.
     * <p>
     * It does nothing if monitoring is not started.
     *
     * @throws RcsDestroyedObjectException
     *             if the object is already destroyed
     *
     * @see #startMonitoring(OnStateChangedListener)
     * @see #isMonitoring()
     */
    public void stopMonitoring() throws RcsException {
        assertAlive();
        nativeStopMonitoring();
    }

    /**
     * Returns the current state of the resource.
     *
     * @return ResourceState - current resource state
     *
     * @throws RcsDestroyedObjectException
     *             if the object is already destroyed
     *
     * @see #startMonitoring(OnStateChangedListener)
     * @see #isMonitoring()
     * @see OnStateChangedListener
     */
    public ResourceState getState() throws RcsException {
        assertAlive();
        return nativeGetState();
    }

    /**
     * Starts caching attributes of the resource.
     *
     * This will start data caching for the resource. Once caching started it
     * will look for the data updation on the resource and updates the cache
     * data accordingly.
     * <p>
     * It is equivalent to calling {@link #startCaching(OnCacheUpdatedListener)}
     * with null.
     *
     * @throws RcsDestroyedObjectException
     *             if the object is already destroyed
     *
     * @see #startCaching(OnCacheUpdatedListener)
     * @see #isCaching()
     * @see #getCacheState()
     * @see #getCachedAttribute(String)
     * @see #getCachedAttributes()
     * @see OnCacheUpdatedListener
     */
    public void startCaching() throws RcsException {
        assertAlive();
        startCaching(null);
    }

    /**
     * Starts caching attributes of the resource.
     *
     * This will start data caching for the resource. Once caching started it
     * will look for the data updation on the resource and updates the cache
     * data accordingly.
     *
     * @param listener
     *            the listener to be notified when attributes are updated or
     *            null if no need
     *
     * @throws RcsDestroyedObjectException
     *             if the object is already destroyed
     *
     * @see #startCaching()
     * @see #isCaching()
     * @see #getCacheState()
     * @see #getCachedAttribute(String)
     * @see #getCachedAttributes()
     * @see OnCacheUpdatedListener
     */
    public void startCaching(OnCacheUpdatedListener listener)
            throws RcsException {
        assertAlive();
        nativeStartCaching(listener);
    }

    /**
     * Stops caching.
     *
     * It does nothing if caching is not started.
     *
     * @throws RcsDestroyedObjectException
     *             if the object is already destroyed
     *
     * @see #startCaching()
     * @see #startCaching(OnCacheUpdatedListener)
     * @see #isCaching()
     *
     */
    public void stopCaching() throws RcsException {
        assertAlive();
        nativeStopCaching();
    }

    /**
     * Returns the current cache state.
     *
     * @return current cache state.
     *
     * @throws RcsDestroyedObjectException
     *             if the object is already destroyed
     *
     * @see #startCaching()
     * @see #startCaching(OnCacheUpdatedListener)
     * @see #isCaching()
     */
    public CacheState getCacheState() throws RcsException {
        assertAlive();
        return nativeGetCacheState();
    }

    /**
     * Returns whether cached data is available.
     *
     * Cache will be available always once cache state had been
     * {@link CacheState#READY} even if current state is
     * {@link CacheState#LOST_SIGNAL} until stopped.
     *
     * @return true if cache data is available.
     *
     * @throws RcsDestroyedObjectException
     *             if the object is already destroyed
     *
     * @see #startCaching()
     * @see #startCaching(OnCacheUpdatedListener)
     * @see #isCaching()
     * @see #getCacheState()
     *
     */
    public boolean isCachedAvailable() throws RcsException {
        assertAlive();
        return nativeIsCachedAvailable();
    }

    /**
     * Returns the cached attributes.
     * <p>
     * Note that this works only when cache is available.
     *
     * @return the cached attributes.
     *
     * @throws RcsIllegalStateException
     *             if cache is not available
     * @throws RcsDestroyedObjectException
     *             if the object is already destroyed
     *
     * @see #startCaching()
     * @see #startCaching(OnCacheUpdatedListener)
     * @see #isCaching()
     * @see #getCacheState()
     * @see #isCachedAvailable()
     * @see #getCachedAttribute(String)
     *
     */
    public RcsResourceAttributes getCachedAttributes() throws RcsException {
        assertAlive();
        return nativeGetCachedAttributes();
    }

    /**
     * Returns the cached value to which the specified key is mapped, or null if
     * no mapping for the key.
     * <p>
     * Note that this works only when cache is available.
     *
     * @param key
     *            the key whose associated value is to be returned
     *
     * @return the value to which the specified key is mapped, or null if no
     *         mapping for the key
     *
     * @throws NullPointerException
     *             if key is null
     * @throws RcsIllegalStateException
     *             if cache is not available
     * @throws RcsDestroyedObjectException
     *             if the object is already destroyed
     *
     * @see #startCaching()
     * @see #startCaching(OnCacheUpdatedListener)
     * @see #isCaching()
     * @see #getCacheState()
     * @see #isCachedAvailable()
     * @see #getCachedAttributes()
     *
     */
    public RcsValue getCachedAttribute(String key) throws RcsException {
        assertAlive();
        if (key == null) {
            throw new NullPointerException("key is null.");
        }

        return getCachedAttributes().get(key);
    }

    /**
     * Sends a request for the resource attributes directly to the resource.
     *
     * @param listener
     *            the listener to receive the response
     *
     * @throws NullPointerException
     *             if listener is null
     * @throws RcsDestroyedObjectException
     *             if the object is already destroyed
     * @throws RcsPlatformException
     *             if the operation failed
     *
     * @see OnRemoteAttributesReceivedListener
     */
    public void getRemoteAttributes(OnRemoteAttributesReceivedListener listener)
            throws RcsException {
        assertAlive();
        if (listener == null) {
            throw new NullPointerException("listener is null.");
        }

        nativeGetRemoteAttributes(listener);
    }

    /**
     * Sends a set request with resource attributes to the resource.
     * <p>
     * The SetRequest behavior depends on the server, whether updating its
     * attributes or not.
     *
     * @param attributes
     *            attributes to set for the remote resource.
     *
     * @throws NullPointerException
     *             if attributes or listener is null
     * @throws RcsDestroyedObjectException
     *             if the object is already destroyed
     * @throws RcsPlatformException
     *             if the operation failed
     *
     * @see OnRemoteAttributesReceivedListener
     */
    public void setRemoteAttributes(RcsResourceAttributes attributes,
            OnRemoteAttributesReceivedListener listener) throws RcsException {
        assertAlive();

        if (attributes == null) {
            throw new NullPointerException("attributes is null.");
        }
        if (listener == null) {
            throw new NullPointerException("listener is null.");
        }

        nativeSetRemoteAttributes(attributes, listener);
    }

    /**
     * Returns the uri of the resource.
     *
     * @return uri of the resource
     *
     * @throws RcsDestroyedObjectException
     *             if the object is already destroyed
     */
    public String getUri() throws RcsException {
        assertAlive();
        return nativeGetUri();
    }

    /**
     * Returns the address of the resource .
     *
     * @return address of the resource
     *
     * @throws RcsDestroyedObjectException
     *             if the object is already destroyed
     */
    public String getAddress() throws RcsException {
        assertAlive();
        return nativeGetAddress();
    }

    /**
     * Returns the resource types of the resource.
     *
     * @return resource types
     *
     * @throws RcsDestroyedObjectException
     *             if the object is already destroyed
     */
    public String[] getTypes() throws RcsException {
        assertAlive();
        return nativeGetTypes();
    }

    /**
     * Returns the resource interfaces of the resource.
     *
     * @return resource interfaces
     *
     * @throws RcsDestroyedObjectException
     *             if the object is already destroyed
     */
    public String[] getInterfaces() throws RcsException {
        assertAlive();
        return nativeGetInterfaces();
    }

    /**
     * Reclaims all resources used by this object.
     * This must be called if the resource is not used any longer.
     *
     */
    public void destroy() {
        super.dispose();
    }
}
