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

import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.iotivity.service.RcsException;
import org.iotivity.service.RcsObject;
import org.iotivity.service.RcsResourceAttributes;
import org.iotivity.service.RcsValue;

public final class RcsLockedAttributes extends RcsObject {

    private static native boolean nativeIsEmpty(RcsResourceObject resourceObj);

    private static native int nativeSize(RcsResourceObject resourceObj);

    private static native boolean nativeRemove(RcsResourceObject resourceObj,
            String key);

    private static native boolean nativeClear(RcsResourceObject resourceObj);

    private static native boolean nativeContains(RcsResourceObject resourceObj,
            String key);

    private static native void nativeAddKeys(RcsResourceObject resourceObj,
            Set<String> set);

    private static native RcsValue nativeAsJavaObject(
            RcsResourceObject resourceObj, String key);

    private static native void nativeApply(RcsResourceObject resourceObj,
            Map<String, RcsValue> cache);

    private native void nativeLock(RcsResourceObject resourceObj);

    private native void nativeUnlock();

    private final RcsResourceObject mResourceObject;

    private boolean mIsUnlocked;

    private Map<String, RcsValue> mCache = new HashMap<>();

    RcsLockedAttributes(RcsResourceObject resourceObject) throws RcsException {
        if (resourceObject == null) {
            throw new RcsException("Illegal opertaion!");
        }

        mResourceObject = resourceObject;

        nativeLock(resourceObject);
    }

    void setUnlockState() {
        mIsUnlocked = true;
        mCache = null;

        nativeUnlock();
    }

    void apply() {
        nativeApply(mResourceObject, mCache);
        mCache.clear();
    }

    private void ensureLocked() throws RcsException {
        if (mIsUnlocked) {
            throw new RcsUnlockedException("This attributes is unlocked!");
        }
    }

    /**
     * Returns a unmodifiable Set view of the keys contained in this attributes.
     *
     * @return an unmodifiable set view of the keys in this attributes
     *
     * @throws RcsUnlockedException
     *             if the {@link RcsResourceObject.AttributesLock} for this
     *             object is unlocked
     */
    public Set<String> keySet() throws RcsException {
        ensureLocked();

        final Set<String> keySet = new HashSet<>(mCache.keySet());

        nativeAddKeys(mResourceObject, keySet);

        return Collections.unmodifiableSet(keySet);
    }

    /**
     * Returns the value to which the specified key is mapped, or null if this
     * contains no mapping for the key.
     *
     * @param key
     *            the key whose associated value is to be returned
     *
     * @return the value to which the specified key is mapped, or null if this
     *         contains no mapping for the key
     *
     * @throws NullPointerException
     *             if key is null
     * @throws RcsUnlockedException
     *             if the {@link RcsResourceObject.AttributesLock} for this
     *             object is unlocked
     */
    public RcsValue get(String key) throws RcsException {
        ensureLocked();

        if (key == null) throw new NullPointerException("key is null");

        if (!mCache.containsKey(key) && nativeContains(mResourceObject, key)) {
            mCache.put(key, nativeAsJavaObject(mResourceObject, key));
        }
        return mCache.get(key);
    }

    /**
     * Copies all of the mappings from the specified to this
     *
     * @param attributes
     *            attributes to be copied
     *
     * @throws RcsUnlockedException
     *             if the {@link RcsResourceObject.AttributesLock} for this
     *             object is unlocked
     *
     */
    public RcsLockedAttributes putAll(RcsResourceAttributes attributes)
            throws RcsException {
        ensureLocked();

        final Set<String> keys = attributes.keySet();

        for (final String k : keys) {
            mCache.put(k, attributes.get(k));
        }
        return this;
    }

    /**
     * Sets the specified value with the specified key.
     * If the object previously contained a mapping for the key, the old value
     * is replaced by the specified value.
     *
     * @param key
     *            key with which the specified value is to be associated
     *
     * @param value
     *            value to be associated with the specified key
     *
     * @throws NullPointerException
     *             if key or value is null
     * @throws RcsUnlockedException
     *             if the {@link RcsResourceObject.AttributesLock} for this
     *             object is unlocked
     *
     */
    public RcsLockedAttributes put(String key, RcsValue value)
            throws RcsException {
        ensureLocked();

        if (key == null) throw new NullPointerException("key is null");
        if (value == null) throw new NullPointerException("value is null");

        mCache.put(key, value);

        return this;
    }

    /**
     * Sets the specified value with the specified key.
     * If the object previously contained a mapping for the key, the old value
     * is replaced by the specified value.
     *
     * @param key
     *            key with which the specified value is to be associated
     *
     * @param value
     *            value to be associated with the specified key
     *
     * @throws NullPointerException
     *             if key or value is null
     * @throws IllegalArgumentException
     *             if object is not supported type by {@link RcsValue}
     * @throws RcsUnlockedException
     *             if the {@link RcsResourceObject.AttributesLock} for this
     *             object is unlocked
     */
    public void put(String key, Object value) throws RcsException {
        if (key == null) throw new NullPointerException("key is null");

        put(key, new RcsValue(value));
    }

    /**
     * Returns whether attribute is empty.
     *
     * @throws RcsUnlockedException
     *             if the {@link RcsResourceObject.AttributesLock} for this
     *             object is unlocked
     */
    public boolean isEmpty() throws RcsException {
        ensureLocked();

        return mCache.isEmpty() && nativeIsEmpty(mResourceObject);
    }

    /**
     * Returns the number of key-value mappings.
     *
     * @throws RcsUnlockedException
     *             if the {@link RcsResourceObject.AttributesLock} for this
     *             object is unlocked
     */
    public int size() throws RcsException {
        ensureLocked();

        return mCache.size() + nativeSize(mResourceObject);
    }

    /**
     * Removes the mapping for a key from this attributes if it is present.
     *
     * @param key
     *            key whose mapping is to be removed
     *
     * @return true if the key is present and the the value mapped is removed.
     *
     * @throws RcsUnlockedException
     *             if the {@link RcsResourceObject.AttributesLock} for this
     *             object is unlocked
     */
    public boolean remove(String key) throws RcsException {
        ensureLocked();

        if (key == null) throw new NullPointerException("key is null");

        // XXX make sure both cache and native values to be removed.
        final boolean cacheRemove = mCache.remove(key) != null;
        final boolean nativeRemove = nativeRemove(mResourceObject, key);

        return cacheRemove || nativeRemove;
    }

    /**
     * Removes all elements.
     *
     * @throws RcsUnlockedException
     *             if the {@link RcsResourceObject.AttributesLock} for this
     *             object is unlocked
     */
    public void clear() throws RcsException {
        ensureLocked();

        nativeClear(mResourceObject);
    }

    /**
     * Returns true if this contains a mapping for the specified key.
     *
     * @param key
     *            key whose presence is to be tested
     *
     * @return true if this contains a mapping for the specified key.
     *
     * @throws NullPointerException
     *             if key is null
     * @throws RcsUnlockedException
     *             if the {@link RcsResourceObject.AttributesLock} for this
     *             object is unlocked
     */
    public boolean contains(String key) throws RcsException {
        ensureLocked();

        if (key == null) throw new NullPointerException("key is null");

        return mCache.containsKey(key) || nativeContains(mResourceObject, key);
    }
}
