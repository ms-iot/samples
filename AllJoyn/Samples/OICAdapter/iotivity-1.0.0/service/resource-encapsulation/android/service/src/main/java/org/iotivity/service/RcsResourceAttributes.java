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

package org.iotivity.service;

import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.iotivity.service.server.RcsLockedAttributes;

/**
 *
 * This class represents the attributes for a resource.
 *
 * @see RcsValue
 */
public final class RcsResourceAttributes extends RcsObject {

    private native boolean nativeIsEmpty();

    private native int nativeSize();

    private native boolean nativeRemove(String key);

    private native void nativeClear();

    private native boolean nativeContains(String key);

    private native void nativeAddKeys(Set<String> set);

    private native RcsValue nativeExtract(String key);

    private native void nativeExtractAll(Map<String, RcsValue> map);

    private final Map<String, RcsValue> mCache = new HashMap<>();

    public RcsResourceAttributes() {
    }

    public RcsResourceAttributes(RcsLockedAttributes lockedAttrs)
            throws RcsException {
        for (final String key : lockedAttrs.keySet()) {
            mCache.put(key, lockedAttrs.get(key));
        }
    }

    /**
     * Returns a unmodifiable Set view of the keys contained in this attributes.
     *
     * @return an unmodifiable set view of the keys in this attributes
     */
    public Set<String> keySet() {
        if (hasHandle()) {
            final Set<String> keySet = new HashSet<>(mCache.keySet());

            nativeAddKeys(keySet);

            return Collections.unmodifiableSet(keySet);
        }

        return Collections.unmodifiableSet(mCache.keySet());
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
     */
    public RcsValue get(String key) {
        if (key == null) throw new NullPointerException("key is null");

        if (!mCache.containsKey(key) && hasHandle() && nativeContains(key)) {
            mCache.put(key, nativeExtract(key));
        }
        return mCache.get(key);
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
     *
     */
    public void put(String key, RcsValue value) {
        if (key == null) throw new NullPointerException("key is null");
        if (value == null) throw new NullPointerException("value is null");

        mCache.put(key, value);
        if (hasHandle()) nativeRemove(key);
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
     */
    public void put(String key, Object object) {
        if (key == null) throw new NullPointerException("key is null");

        put(key, new RcsValue(object));
    }

    /**
     * Returns true if this contains no key-value mappings.
     *
     * @return true if this contains no key-value mappings
     */
    public boolean isEmpty() {
        return mCache.isEmpty() && (!hasHandle() || nativeIsEmpty());
    }

    /**
     * Returns the number of key-value mappings.
     *
     * @return the number of key-value mappings
     */
    public int size() {
        if (hasHandle()) return mCache.size() + nativeSize();
        return mCache.size();
    }

    /**
     * Removes the mapping for a key from this attributes if it is present.
     *
     * @param key
     *            key whose mapping is to be removed
     *
     * @return true if the key is present and the the value mapped is removed.
     */
    public boolean remove(String key) {
        if (key == null) throw new NullPointerException("key is null");

        // XXX make sure both cache and native values to be removed.
        final boolean cacheRemove = mCache.remove(key) != null;
        final boolean nativeRemove = hasHandle() && nativeRemove(key);

        return cacheRemove || nativeRemove;
    }

    /**
     * Removes all of the mappings.
     */
    public void clear() {
        mCache.clear();
        nativeClear();
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
     */
    public boolean contains(String key) {
        if (key == null) throw new NullPointerException("key is null");

        return mCache.containsKey(key) || nativeContains(key);
    }

    private void esnureAllExtracted() {
        if (hasHandle()) nativeExtractAll(mCache);
    }

    @Override
    public boolean equals(Object o) {
        if (o == this) return true;
        if (!(o instanceof RcsResourceAttributes)) return false;

        final RcsResourceAttributes rhs = (RcsResourceAttributes) o;

        esnureAllExtracted();
        rhs.esnureAllExtracted();

        return mCache.equals(rhs.mCache);
    }

    @Override
    public int hashCode() {
        esnureAllExtracted();
        return mCache.hashCode();
    }

}
