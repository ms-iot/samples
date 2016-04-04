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
import java.util.Map;

/**
 * Value holds a value among various types at a time.
 *
 * Type helps identify type information of Value.
 *
 * @see RcsResourceAttributes
 * @see Type
 */
public final class RcsValue {

    private static class NullType {
        @Override
        public String toString() {
            return "";
        }
    }

    /**
     * Identifiers for types of Value.
     *
     * @see Type
     */
    public static enum TypeId {
        NULL, BOOLEAN, INTEGER, DOUBLE, STRING, ATTRIBUTES, ARRAY;
    }

    /**
     * A Helper class to identify types of Value.
     *
     * @see RcsResourceAttributes
     * @see RcsValue
     * @see TypeId
     */
    public static class Type {
        private final TypeId mTypeId;

        Type(TypeId typeId) {
            mTypeId = typeId;
        }

        /**
         * Returns type identifier.
         *
         * @return Identifier of type
         *
         * @see #getBaseTypeId(RcsValue.Type)
         */
        public final TypeId getId() {
            return mTypeId;
        }

        protected TypeId getBaseTypeId() {
            return mTypeId;
        }

        protected int getDepth() {
            return 0;
        }

        /**
         * Returns the type identifier of a base type of sequence.
         *
         * For non sequence types, it is equivalent to calling {@link #getId()}.
         *
         * @return identifier of type
         *
         * @see getDepth
         * @see getId
         */
        public static TypeId getBaseTypeId(Type t) {
            return t.getBaseTypeId();
        }

        /**
         * Returns the depth of a type.
         *
         * The return will be zero for non sequence types.
         *
         * @see getBaseTypeId
         */
        public static int getDepth(Type t) {
            return t.getDepth();
        }

        /**
         * Factory method to create Type instance from an object.
         * Note that object must be a supported type by RcsValue.
         *
         * @return An instance that has TypeId for obj.
         *
         * @throws NullPointerException
         *             if obj is null.
         * @throws IllegalArgumentException
         *             if obj is not supported type.
         *
         */
        public static Type typeOf(Object obj) {
            if (obj == null) {
                throw new NullPointerException("object is null");
            }

            return typeOf(obj.getClass());
        }

        /**
         * Factory method to create Type instance from a class.
         * Note that class must be a supported type by RcsValue.
         *
         * @return An instance that has TypeId for class.
         *
         * @throws NullPointerException
         *             if cls is null.
         * @throws IllegalArgumentException
         *             if cls is not supported type.
         *
         */
        public static Type typeOf(Class<?> cls) {
            if (cls == null) {
                throw new NullPointerException("class is null");
            }

            if (sTypes.containsKey(cls)) return sTypes.get(cls);

            throw new IllegalArgumentException(
                    cls.getSimpleName() + " is not supported type.");
        }
    }

    private static class ArrayType extends Type {
        private final TypeId mBaseTypeId;
        private final int    mDimension;

        ArrayType(TypeId baseTypeId, int dimension) {
            super(TypeId.ARRAY);

            mBaseTypeId = baseTypeId;
            mDimension = dimension;
        }

        @Override
        protected TypeId getBaseTypeId() {
            return mBaseTypeId;
        }

        @Override
        protected int getDepth() {
            return mDimension;
        }
    }

    private static final NullType sNullValue = new NullType();

    private static Map<Class<?>, Type> sTypes;

    private final Object mObject;
    private Type         mType;

    static {
        final Map<Class<?>, Type> types = new HashMap<Class<?>, Type>();

        types.put(NullType.class, new Type(TypeId.NULL));
        types.put(Boolean.class, new Type(TypeId.BOOLEAN));
        types.put(Integer.class, new Type(TypeId.INTEGER));
        types.put(Double.class, new Type(TypeId.DOUBLE));
        types.put(String.class, new Type(TypeId.STRING));
        types.put(RcsResourceAttributes.class, new Type(TypeId.ATTRIBUTES));

        types.put(boolean[].class, new ArrayType(TypeId.BOOLEAN, 1));
        types.put(int[].class, new ArrayType(TypeId.INTEGER, 1));
        types.put(double[].class, new ArrayType(TypeId.DOUBLE, 1));
        types.put(String[].class, new ArrayType(TypeId.STRING, 1));
        types.put(RcsResourceAttributes[].class,
                new ArrayType(TypeId.ATTRIBUTES, 1));

        types.put(boolean[][].class, new ArrayType(TypeId.BOOLEAN, 2));
        types.put(int[][].class, new ArrayType(TypeId.INTEGER, 2));
        types.put(double[][].class, new ArrayType(TypeId.DOUBLE, 2));
        types.put(String[][].class, new ArrayType(TypeId.STRING, 2));
        types.put(RcsResourceAttributes[][].class,
                new ArrayType(TypeId.ATTRIBUTES, 2));

        types.put(boolean[][][].class, new ArrayType(TypeId.BOOLEAN, 3));
        types.put(int[][][].class, new ArrayType(TypeId.INTEGER, 3));
        types.put(double[][][].class, new ArrayType(TypeId.DOUBLE, 3));
        types.put(String[][][].class, new ArrayType(TypeId.STRING, 3));
        types.put(RcsResourceAttributes[][][].class,
                new ArrayType(TypeId.ATTRIBUTES, 3));

        sTypes = Collections.unmodifiableMap(types);

    }

    static boolean isSupportedType(Class<?> cls) {
        return sTypes.containsKey(cls);
    }

    static void verifySupportedType(Class<?> cls) {
        if (!isSupportedType(cls)) {
            throw new IllegalArgumentException(
                    cls.getSimpleName() + " is not supported type.");
        }
    }

    /**
     * Constructs a new value with an object.
     *
     * @param value
     *            An object
     *
     * @throws NullPointerException
     *             if value is null.
     * @throws IllegalArgumentException
     *             if value is not supported type.
     */
    public RcsValue(Object value) {
        if (value == null) throw new NullPointerException("value is null!");

        verifySupportedType(value.getClass());

        mObject = value;
    }

    /**
     * Constructs a new value that holds null value.
     *
     */
    public RcsValue() {
        this(sNullValue);
    }

    /**
     * Constructs a new value that holds a boolean value.
     *
     * @param value
     *            a boolean
     */
    public RcsValue(boolean value) {
        this((Object) value);
    }

    /**
     * Constructs a new value that holds an int value.
     *
     * @param value
     *            an int
     */
    public RcsValue(int value) {
        this((Object) value);
    }

    /**
     * Constructs a new value that holds a double value.
     *
     * @param value
     *            a double
     */
    public RcsValue(double value) {
        this((Object) value);
    }

    /**
     * Constructs a new value that holds a String value.
     *
     * @param value
     *            a String
     *
     * @throws NullPointerException
     *             if value is null.
     */
    public RcsValue(String value) {
        this((Object) value);
    }

    /**
     * Constructs a new value that holds a RcsResourceAttributes value.
     *
     * @param value
     *            a RcsResourceAttributes
     *
     * @throws NullPointerException
     *             if value is null.
     */
    public RcsValue(RcsResourceAttributes value) {
        this((Object) value);
    }

    /**
     * Constructs a new value that holds a boolean array.
     *
     * @param value
     *            a boolean array
     *
     * @throws NullPointerException
     *             if value is null.
     */
    public RcsValue(boolean[] value) {
        this((Object) value);
    }

    /**
     * Constructs a new value that holds a two-dimensional boolean array.
     *
     * @param value
     *            a two-dimensional boolean array
     *
     * @throws NullPointerException
     *             if value is null.
     */
    public RcsValue(boolean[][] value) {
        this((Object) value);
    }

    /**
     * Constructs a new value that holds a three-dimensional boolean array.
     *
     * @param value
     *            a three-dimensional boolean array
     *
     * @throws NullPointerException
     *             if value is null.
     */
    public RcsValue(boolean[][][] value) {
        this((Object) value);
    }

    /**
     * Constructs a new value that holds an int array.
     *
     * @param value
     *            an int array
     *
     * @throws NullPointerException
     *             if value is null.
     */
    public RcsValue(int[] value) {
        this((Object) value);
    }

    /**
     * Constructs a new value that holds a two-dimensional int array.
     *
     * @param value
     *            a two-dimensional int array
     *
     * @throws NullPointerException
     *             if value is null.
     */
    public RcsValue(int[][] value) {
        this((Object) value);
    }

    /**
     * Constructs a new value that holds a three-dimensional int array.
     *
     * @param value
     *            a three-dimensional int array
     *
     * @throws NullPointerException
     *             if value is null.
     */
    public RcsValue(int[][][] value) {
        this((Object) value);
    }

    /**
     * Constructs a new value that holds a double array.
     *
     * @param value
     *            a double array
     *
     * @throws NullPointerException
     *             if value is null.
     */
    public RcsValue(double[] value) {
        this((Object) value);
    }

    /**
     * Constructs a new value that holds a two-dimensional double array.
     *
     * @param value
     *            a two-dimensional double array
     *
     * @throws NullPointerException
     *             if value is null.
     */
    public RcsValue(double[][] value) {
        this((Object) value);
    }

    /**
     * Constructs a new value that holds a three-dimensional double array.
     *
     * @param value
     *            a three-dimensional double array
     *
     * @throws NullPointerException
     *             if value is null.
     */
    public RcsValue(double[][][] value) {
        this((Object) value);
    }

    /**
     * Constructs a new value that holds a String array.
     *
     * @param value
     *            a String array
     *
     * @throws NullPointerException
     *             if value is null.
     */
    public RcsValue(String[] value) {
        this((Object) value);
    }

    /**
     * Constructs a new value that holds a two-dimensional String array.
     *
     * @param value
     *            a two-dimensional String array
     *
     * @throws NullPointerException
     *             if value is null.
     */
    public RcsValue(String[][] value) {
        this((Object) value);
    }

    /**
     * Constructs a new value that holds a three-dimensional String array.
     *
     * @param value
     *            a three-dimensional String array
     *
     * @throws NullPointerException
     *             if value is null.
     */
    public RcsValue(String[][][] value) {
        this((Object) value);
    }

    /**
     * Constructs a new value that holds a RcsResourceAttributes array.
     *
     * @param value
     *            a RcsResourceAttributes array
     *
     * @throws NullPointerException
     *             if value is null.
     */
    public RcsValue(RcsResourceAttributes[] value) {
        this((Object) value);
    }

    /**
     * Constructs a new value that holds a two-dimensional RcsResourceAttributes
     * array.
     *
     * @param value
     *            a two-dimensional RcsResourceAttributes array
     *
     * @throws NullPointerException
     *             if value is null.
     */
    public RcsValue(RcsResourceAttributes[][] value) {
        this((Object) value);
    }

    /**
     * Constructs a new value that holds a three-dimensional
     * RcsResourceAttributes array.
     *
     * @param value
     *            a three-dimensional RcsResourceAttributes array
     *
     * @throws NullPointerException
     *             if value is null.
     */
    public RcsValue(RcsResourceAttributes[][][] value) {
        this((Object) value);
    }

    /**
     * Returns whether the value is null.
     *
     * @return true if the value is null.
     */
    public boolean isNull() {
        return isNullObject(mObject);
    }

    /**
     * Returns whether the object represents null for RcsValue.
     *
     * @param o
     *            an object to be tested
     *
     * @return true if the object represents null.
     */
    public static boolean isNullObject(Object o) {
        return o == sNullValue;
    }

    /**
     * Returns type information.
     *
     * @return type information for the value.
     */
    public Type getType() {
        if (mType == null) mType = Type.typeOf(mObject);
        return mType;
    }

    /**
     * Returns the value as T.
     *
     * @return a value as T
     *
     * @throws ClassCastException
     *             if the value is not of T.
     */
    @SuppressWarnings("unchecked")
    public <T> T get() {
        return (T) mObject;
    }

    @SuppressWarnings("unchecked")
    private <T> T getOrNull() {
        try {
            return (T) mObject;
        } catch (final ClassCastException e) {
            return null;
        }
    }

    /**
     * Returns the value as an Object.
     *
     * @return an Object
     */
    public Object asObject() {
        return mObject;
    }

    /**
     * Returns the value as a boolean, false if the value is not the desired
     * type.
     *
     * @return a boolean value
     *
     */
    public boolean asBoolean() {
        return asBoolean(false);
    }

    /**
     * Returns the value as a boolean.
     *
     * @param defaultValue
     *            value to return if the value is not boolean.
     *
     * @return a boolean value
     *
     */
    public boolean asBoolean(boolean defaultValue) {
        try {
            return get();
        } catch (final ClassCastException e) {
            return defaultValue;
        }
    }

    /**
     * Returns the value as an int, 0 if the value is not the desired type.
     *
     * @return an int value
     *
     */
    public int asInt() {
        return asInt(0);
    }

    /**
     * Returns the value as an int.
     *
     * @param defaultValue
     *            value to return if the value is not int.
     *
     * @return an int value
     *
     */
    public int asInt(int defaultValue) {
        try {
            return get();
        } catch (final ClassCastException e) {
            return defaultValue;
        }
    }

    /**
     * Returns the value as a double, 0 if the value is not the desired type.
     *
     * @return a double value
     *
     */
    public double asDouble() {
        return asDouble(0);
    }

    /**
     * Returns the value as a double.
     *
     * @param defaultValue
     *            value to return if the value is not double.
     *
     * @return a double value
     *
     */
    public double asDouble(double defaultValue) {
        try {
            return get();
        } catch (final ClassCastException e) {
            return defaultValue;
        }
    }

    /**
     * Returns the value as a string, null if the value is not the desired type.
     *
     * @return a string value
     *
     */
    public String asString() {
        return asString(null);
    }

    /**
     * Returns the value as a String.
     *
     * @param defaultValue
     *            value to return if the value is not String.
     *
     * @return a String value
     *
     */
    public String asString(String defaultValue) {
        try {
            return get();
        } catch (final ClassCastException e) {
            return defaultValue;
        }
    }

    /**
     * Returns the value as a RcsResourceAttributes,
     * null if the value is not the desired type.
     *
     * @return a RcsResourceAttributes value
     *
     */
    public RcsResourceAttributes asAttributes() {
        return getOrNull();
    }

    /**
     * Returns the value as a boolean array, null if the value is not the
     * desired type.
     *
     * @return a boolean array
     *
     */
    public boolean[] asBooleanArray() {
        return getOrNull();
    }

    /**
     * Returns the value as a two-dimensional boolean array, null if the value
     * is not the desired type.
     *
     * @return a two-dimensional boolean array
     *
     */
    public boolean[][] asBoolean2DArray() {
        return getOrNull();
    }

    /**
     * Returns the value as a three-dimensional boolean array, null if the value
     * is not the desired type.
     *
     * @return a three-dimensional boolean array
     *
     */
    public boolean[][][] asBoolean3DArray() {
        return getOrNull();
    }

    /**
     * Returns the value as an int array, null if the value is not the
     * desired type.
     *
     * @return an int array
     *
     */
    public int[] asIntArray() {
        return getOrNull();
    }

    /**
     * Returns the value as a two-dimensional int array, null if the value
     * is not the desired type.
     *
     * @return a two-dimensional int array
     *
     */
    public int[][] asInt2DArray() {
        return getOrNull();
    }

    /**
     * Returns the value as a three-dimensional int array, null if the value
     * is not the desired type.
     *
     * @return a three-dimensional int array
     *
     */
    public int[][][] asInt3DArray() {
        return getOrNull();
    }

    /**
     * Returns the value as a double array, null if the value is not the
     * desired type.
     *
     * @return a double array
     *
     */
    public double[] asDoubleArray() {
        return getOrNull();
    }

    /**
     * Returns the value as a two-dimensional double array, null if the value
     * is not the desired type.
     *
     * @return a two-dimensional double array
     *
     */
    public double[][] asDouble2DArray() {
        return getOrNull();
    }

    /**
     * Returns the value as a three-dimensional double array, null if the value
     * is not the desired type.
     *
     * @return a three-dimensional double array
     *
     */
    public double[][][] asDouble3DArray() {
        return getOrNull();
    }

    /**
     * Returns the value as a string array, null if the value is not the
     * desired type.
     *
     * @return a string array
     *
     */
    public String[] asStringArray() {
        return getOrNull();
    }

    /**
     * Returns the value as a two-dimensional string array, null if the value
     * is not the desired type.
     *
     * @return a two-dimensional string array
     *
     */
    public String[][] asString2DArray() {
        return getOrNull();
    }

    /**
     * Returns the value as a three-dimensional string array, null if the value
     * is not the desired type.
     *
     * @return a three-dimensional string array
     *
     */
    public String[][][] asString3DArray() {
        return getOrNull();
    }

    /**
     * Returns the value as an attributes array, null if the value is not the
     * desired type.
     *
     * @return an attributes array
     *
     */
    public RcsResourceAttributes[] asAttributesArray() {
        return getOrNull();
    }

    /**
     * Returns the value as a two-dimensional attributes array, null if the
     * value is not the desired type.
     *
     * @return a two-dimensional attributes array
     *
     */
    public RcsResourceAttributes[][] asAttributes2DArray() {
        return getOrNull();
    }

    /**
     * Returns the value as a three-dimensional attributes array, null if the
     * value is not the desired type.
     *
     * @return a three-dimensional attributes array
     *
     */
    public RcsResourceAttributes[][][] asAttributes3DArray() {
        return getOrNull();
    }

    @Override
    public boolean equals(Object o) {
        if (o == this) return true;
        if (!(o instanceof RcsValue)) return false;

        final RcsValue rhs = (RcsValue) o;

        return mObject.equals(rhs.mObject);
    }

    @Override
    public int hashCode() {
        return mObject.hashCode();
    }

    @Override
    public String toString() {
        return mObject.toString();
    }

}
