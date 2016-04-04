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
/**
 * @file   RCSResourceObject.java
 *
 *  This file contains the resource object APIs provided to the developers.
 *        RCSResourceObject is a part of the server builder module.
 *
 */

package org.iotivity.service.server;

import java.lang.ref.WeakReference;

import org.iotivity.service.RcsDestroyedObjectException;
import org.iotivity.service.RcsException;
import org.iotivity.service.RcsIllegalStateException;
import org.iotivity.service.RcsObject;
import org.iotivity.service.RcsPlatformException;
import org.iotivity.service.RcsResourceAttributes;
import org.iotivity.service.RcsValue;

/**
 * RCSResourceObject represents a resource. It handles any requests from clients
 * automatically with attributes.
 * <p>
 * It also provides an auto notification mechanism that notifies to the
 * observers. Requests are handled automatically by defaultAction of
 * {@link RcsGetResponse} and {@link RcsSetResponse}. You can override them and
 * send your own response with {@link GetRequestHandler} and
 * {@link SetRequestHandler}.
 * <p>
 * For simple resources, they are simply required to notify whenever attributes
 * are changed by a set request. In this case, add an
 * {@link OnAttributeUpdatedListener} with a key interested in instead of
 * overriding {@link SetRequestHandler}.
 *
 * @see Builder
 */
public final class RcsResourceObject extends RcsObject {
    /**
     * This is a builder to create resource with properties and attributes.
     *
     * The resource will be observable and discoverable by default, to make them
     * disable
     * set these properties explicitly with setDiscoverable and setObservable.
     *
     */
    public static class Builder {
        private final String          mUri;
        private final String          mType;
        private final String          mInterface;
        private boolean               mIsObservable    = true;
        private boolean               mIsDiscovervable = true;
        private RcsResourceAttributes mAttributes;

        /**
         * Constructs a Builder.
         *
         * @param uri
         *            resource uri
         * @param resourceType
         *            resource type
         * @param resourceInterface
         *            resource interface
         *
         * @throws NullPointerException
         *             if any parameter is null
         */
        public Builder(String uri, String resourceType,
                String resourceInterface) {
            if (uri == null) {
                throw new NullPointerException("uri is null.");
            }
            if (resourceType == null) {
                throw new NullPointerException("resourceType is null.");
            }
            if (resourceInterface == null) {
                throw new NullPointerException("resourceInterface is null.");
            }

            mUri = uri;
            mType = resourceType;
            mInterface = resourceInterface;
        }

        /**
         * Sets whether the resource is discoverable.
         *
         * @param isDiscoverable
         *            whether to be discoverable or not
         *
         */
        public Builder setDiscoverable(boolean isDiscoverable) {
            mIsDiscovervable = isDiscoverable;
            return this;
        }

        /**
         * Sets the observable(OC_OBSERVABLE) property of the resource.
         *
         * @param isObservable
         *            whether to be observable or not
         *
         */
        public Builder setObservable(boolean isObservable) {
            mIsObservable = isObservable;
            return this;
        }

        /**
         * Sets attributes foe the resource.
         *
         */
        public Builder setAttributes(RcsResourceAttributes attributes) {
            mAttributes = attributes;
            return this;
        }

        /**
         * Register a resource and returns a RCSResourceObject.
         *
         * @throws RcsPlatformException
         *             If registering a resource is failed.
         *
         */
        public RcsResourceObject build() {
            return nativeBuild(mUri, mType, mInterface, mIsObservable,
                    mIsDiscovervable, mAttributes);
        }
    }

    /**
     * This provides the way to get the attributes of RcsResourceObject with
     * lock.
     * When a thread holds the lock, the other threads will be pending until the
     * lock is released by unlock.
     *
     * Here is the standard idiom for AttributesLock:
     *
     * <pre>
     * {@code
     * AttributesLock lock = rcsResourceObject.getAttributesLock();
     *
     * try {
     *     lock.lock();
     *
     *     ....
     *
     *     lock.apply();
     * } finally {
     *     lock.unlock();
     * }
     * }
     * </pre>
     */
    public static class AttributesLock {

        private final WeakReference<RcsResourceObject> mResourceObjectRef;

        private RcsLockedAttributes mCurrentAttributes;

        private AttributesLock(RcsResourceObject resourceObj) {
            mResourceObjectRef = new WeakReference<RcsResourceObject>(
                    resourceObj);
        }

        private RcsResourceObject ensureResourceObject() throws RcsException {
            final RcsResourceObject object = mResourceObjectRef.get();

            if (object == null || object.isDestroyed()) {
                throw new RcsDestroyedObjectException(
                        "The object is already destroyed!");
            }

            return object;
        }

        /**
         * Locks the attributes of the RcsResourceObject and returns locked
         * attributes that can be modified until unlocked.
         *
         * @return Locked attributes.
         *
         * @throws RcsException
         *             if the RcsResourceObject is destroyed
         */
        public RcsLockedAttributes lock() throws RcsException {
            return mCurrentAttributes = new RcsLockedAttributes(
                    ensureResourceObject());
        }

        /**
         * Changes the state to unlock of the attributes of the
         * RcsResourceObject.
         *
         */
        public void unlock() {
            if (mCurrentAttributes == null) return;

            mCurrentAttributes.setUnlockState();
            mCurrentAttributes = null;
        }

        /**
         * Applies the modified attributes to the RcsResourceObject.
         *
         * @throws RcsIllegalStateException
         *             if not in locked state
         */
        public void apply() throws RcsIllegalStateException {
            if (mCurrentAttributes == null) {
                throw new RcsIllegalStateException("it is not locked state.");
            }
            mCurrentAttributes.apply();
        }
    }

    private static native RcsResourceObject nativeBuild(String uri,
            String resourceType, String resourceInterface, boolean isObservable,
            boolean isDiscoverable, RcsResourceAttributes attributes);

    private native void nativeSetAttribute(String key, RcsValue value);

    private native RcsValue nativeGetAttributeValue(String key);

    private native boolean nativeRemoveAttribute(String key);

    private native boolean nativeContainsAttribute(String key);

    private native RcsResourceAttributes nativeGetAttributes();

    private native boolean nativeIsObservable();

    private native boolean nativeIsDiscoverable();

    private native void nativeNotify();

    private native void nativeSetAutoNotifyPolicy(AutoNotifyPolicy policy);

    private native AutoNotifyPolicy nativeGetAutoNotifyPolicy();

    private native void nativeSetSetRequestHandlerPolicy(
            SetRequestHandlerPolicy policy);

    private native SetRequestHandlerPolicy nativeGetSetRequestHandlerPolicy();

    private native void nativeSetGetRequestHandler(GetRequestHandler handler);

    private native void nativeSetSetRequestHandler(SetRequestHandler handler);

    private native void nativeAddAttributeUpdatedListener(String key,
            OnAttributeUpdatedListener listener);

    private native boolean nativeRemoveAttributeUpdatedListener(String key);

    private RcsResourceObject() {
    }

    /**
     * Represents the policy of AutoNotify function of RCSResourceObject class
     * In accord with this, observers are notified of attributes that are
     * changed or updated.
     *
     * <p>
     * Attributes are changed or updated according to execution of some
     * functions which modify attributes or receipt of set requests.
     *
     * @see setAttribute
     * @see removeAttribute
     * @see getAttributesLock
     *
     */
    public enum AutoNotifyPolicy {
        /** Never */
        NEVER,

        /** Always */
        ALWAYS,

        /** When attributes are changed */
        UPDATED
    }

    /**
     * Represents the policy of set-request handler.
     * In accord with this, the RCSResourceObject decides whether a set-request
     * is
     * acceptable or not.
     */
    public enum SetRequestHandlerPolicy {
        /**
         * Requests will be ignored if attributes of the request contain
         * a new key or a value that has different type from the current
         * value of the key.
         */
        NEVER,

        /**
         * The attributes of the request will be applied unconditionally
         * even if there are new name or type conflicts.
         */
        ACCEPT
    }

    /**
     * Interface definition for a handler to be invoked when a get request is
     * received.
     * <p>
     * The handler will be called first when a get request is received, before
     * the RCSResourceObject handles.
     *
     * @see setGetRequestHandler
     */
    public interface GetRequestHandler {

        /**
         * Called when received a get request from the client.
         *
         * @param request
         *            Request information.
         * @param attributes
         *            The attributes of the request.
         *
         * @return A response to be sent.
         *
         * @see RcsGetResponse
         */
        RcsGetResponse onGetRequested(RcsRequest request,
                RcsResourceAttributes attributes);

    }

    /**
     * Interface definition for a handler to be invoked when a set request is
     * received.
     * <p>
     * The handler will be called first when a get request is received, before
     * the RCSResourceObject handles. If the attributes are modified in the
     * callback, the modified attributes will be set in the RCSResourceObject if
     * the request is not ignored.
     *
     * @see setGetRequestHandler
     */
    public interface SetRequestHandler {

        /**
         * Called when received a set request from the client.
         *
         * @param request
         *            request information
         * @param attributes
         *            the attributes of the request.
         *            it will be applied to the RcsResourceObject
         *
         * @return A response indicating how to handle this request.
         *
         * @see RcsSetResponse
         */
        RcsSetResponse onSetRequested(RcsRequest request,
                RcsResourceAttributes attributes);

    }

    /**
     * Interface definition for a callback to be invoked when an attribute is
     * updated.
     */
    public interface OnAttributeUpdatedListener {

        /**
         * Called when an attribute value is updated.
         *
         * @param oldValue
         *            the attribute value before updated
         * @param newValue
         *            the current resource attribute value
         */
        void onAttributeUpdated(RcsValue oldValue, RcsValue newValue);
    }

    private void assertAlive() throws RcsException {
        if (!hasHandle()) {
            throw new RcsDestroyedObjectException(
                    "The object is already destroyed!");
        }
    }

    /**
     * Sets a particular attribute value.
     *
     * @param key
     *            key with which the specified value is to be associated
     * @param value
     *            value to be associated with the specified key
     *
     * @throws RcsDestroyedObjectException
     *             if the object is destroyed
     * @throws NullPointerException
     *             if key or value is null
     *
     */
    public void setAttribute(String key, RcsValue value) throws RcsException {
        assertAlive();

        if (key == null) throw new NullPointerException("key is null");
        if (value == null) throw new NullPointerException("value is null");

        nativeSetAttribute(key, value);
    }

    /**
     * Returns a copied attribute value associated with the supplied key.
     *
     * @param key
     *            the key whose associated value is to be returned
     *
     * @return the value to which the specified key is mapped, or null if no
     *         attribute for the key
     *
     * @throws RcsDestroyedObjectException
     *             if the object is destroyed
     * @throws NullPointerException
     *             if key is null
     */
    public RcsValue getAttributeValue(String key) throws RcsException {
        assertAlive();

        if (key == null) throw new NullPointerException("key is null");
        return nativeGetAttributeValue(key);
    }

    /**
     * Removes the mapping for a key from the attributes if it is present.
     *
     * @param key
     *            key whose mapping is to be removed
     *
     * @return true if the key is present and the the value mapped is removed.
     *
     * @throws RcsDestroyedObjectException
     *             if the object is destroyed
     * @throws NullPointerException
     *             if key is null
     */
    public boolean removeAttribute(String key) throws RcsException {
        assertAlive();

        if (key == null) throw new NullPointerException("key is null");
        return nativeRemoveAttribute(key);
    }

    /**
     * Returns true if the attributes contains a mapping for the specified key.
     *
     * @param key
     *            key whose presence is to be tested
     *
     * @return true if the attributes contains a mapping for the specified key.
     *
     * @throws RcsDestroyedObjectException
     *             if the object is destroyed
     * @throws NullPointerException
     *             if key is null
     */
    public boolean containsAttribute(String key) throws RcsException {
        assertAlive();

        if (key == null) throw new NullPointerException("key is null");
        return nativeContainsAttribute(key);
    }

    /**
     * Returns a copied attributes of the RCSResourceObject.
     * To modify the attributes, use {@link AttributesLock}.
     *
     * @throws RcsDestroyedObjectException
     *             if the object is destroyed
     *
     * @see getAttributesLock
     */
    public RcsResourceAttributes getAttributes() throws RcsException {
        assertAlive();

        return nativeGetAttributes();
    }

    /**
     * Returns an AttributesLock for this RcsResourceObject.
     *
     * @throws RcsDestroyedObjectException
     *             if the object is destroyed
     */
    public AttributesLock getAttributesLock() throws RcsException {
        assertAlive();

        return new AttributesLock(this);
    }

    /**
     * Checks whether the resource is observable or not.
     *
     * @throws RcsDestroyedObjectException
     *             if the object is destroyed
     */
    public boolean isObservable() throws RcsException {
        assertAlive();

        return nativeIsObservable();
    }

    /**
     * Checks whether the resource is discoverable or not.
     *
     * @throws RcsDestroyedObjectException
     *             if the object is destroyed
     */
    public boolean isDiscoverable() throws RcsException {
        assertAlive();

        return nativeIsDiscoverable();
    }

    /**
     * Sets the get request handler. To remove handler, pass null.
     *
     * Default behavior is {@link RcsGetResponse#defaultAction()}.
     *
     * @throws RcsDestroyedObjectException
     *             if the object is destroyed
     */
    public void setGetRequestHandler(GetRequestHandler handler)
            throws RcsException {
        assertAlive();

        nativeSetGetRequestHandler(handler);
    }

    /**
     * Sets the set request handler. To remove handler, pass null.
     *
     * Default behavior is {@link RcsSetResponse#defaultAction()}.
     *
     * @throws RcsDestroyedObjectException
     *             if the object is destroyed
     *
     */
    public void setSetRequestHandler(SetRequestHandler handler)
            throws RcsException {
        assertAlive();

        nativeSetSetRequestHandler(handler);
    }

    /**
     * Adds a listener for a particular attribute updated.
     *
     * @param key
     *            the interested attribute's key
     * @param listener
     *            listener to be invoked
     *
     * @throws NullPointerException
     *             if key or listener is null
     * @throws RcsDestroyedObjectException
     *             if the object is destroyed
     */
    public void addAttributeUpdatedListener(String key,
            OnAttributeUpdatedListener listener) throws RcsException {
        assertAlive();

        if (key == null) {
            throw new NullPointerException("key is null.");
        }
        if (listener == null) {
            throw new NullPointerException("listener is null.");
        }

        nativeAddAttributeUpdatedListener(key, listener);
    }

    /**
     * Removes a listener for a particular attribute updated.
     *
     * @param key
     *            key the key associated with the listener to be removed
     *
     * @return true if the listener added with same key exists and is removed.
     *
     * @throws RcsDestroyedObjectException
     *             if the object is destroyed
     * @throws NullPointerException
     *             if key is null
     */
    public boolean removeAttributeUpdatedListener(String key)
            throws RcsException {
        assertAlive();

        if (key == null) throw new NullPointerException("key is null");
        return nativeRemoveAttributeUpdatedListener(key);
    }

    /**
     * Notifies all observers of the current attributes.
     *
     * @throws RcsDestroyedObjectException
     *             if the object is destroyed
     * @throws RcsPlatformException
     *             if the operation failed
     */
    public void notifyObservers() throws RcsException {
        assertAlive();

        nativeNotify();
    }

    /**
     * Sets auto notify policy
     *
     * @param policy
     *            policy to be set
     *
     * @throws RcsDestroyedObjectException
     *             if the object is destroyed
     *
     */
    public void setAutoNotifyPolicy(AutoNotifyPolicy policy)
            throws RcsException {
        assertAlive();

        if (policy == null) throw new NullPointerException("policy is null");
        nativeSetAutoNotifyPolicy(policy);
    }

    /**
     * Returns the current policy
     *
     * @throws RcsDestroyedObjectException
     *             if the object is destroyed
     *
     */
    public AutoNotifyPolicy getAutoNotifyPolicy() throws RcsException {
        assertAlive();

        return nativeGetAutoNotifyPolicy();
    }

    /**
     * Sets the policy for handling a set request.
     *
     * @param policy
     *            policy to be set
     *
     * @throws RcsDestroyedObjectException
     *             if the object is destroyed
     *
     */
    public void setSetRequestHandlerPolicy(SetRequestHandlerPolicy policy)
            throws RcsException {
        assertAlive();

        if (policy == null) throw new NullPointerException("policy is null");
        nativeSetSetRequestHandlerPolicy(policy);
    }

    /**
     * Returns the current policy.
     *
     * @throws RcsDestroyedObjectException
     *             if the object is destroyed
     */
    public SetRequestHandlerPolicy getSetRequestHandlerPolicy()
            throws RcsException {
        assertAlive();

        return nativeGetSetRequestHandlerPolicy();
    }

    private boolean isDestroyed() {
        return !hasHandle();
    }

    /**
     * Unregister the resource and reclaims all resources used by this object.
     * This must be called if the resource is not used any longer.
     *
     */
    public void destroy() {
        super.dispose();
    }
}
