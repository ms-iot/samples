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

import java.security.InvalidParameterException;
import java.util.Arrays;
import java.util.List;

/**
 *
 */
public class OcRepresentation {

    static {
        System.loadLibrary("oc");
        System.loadLibrary("ocstack-jni");
    }

    public OcRepresentation() {
        create();
        //Native OCRepresentation object was created using "new" and needs to be deleted
        this.mNativeNeedsDelete = true;
    }

    private OcRepresentation(long nativeHandle) {
        this.mNativeHandle = nativeHandle;
        this.mNativeNeedsDelete = false;
    }

    private OcRepresentation(long nativeHandle, boolean nativeNeedsDelete) {
        this.mNativeHandle = nativeHandle;
        this.mNativeNeedsDelete = nativeNeedsDelete;
    }

    public <T> T getValue(String key) throws OcException {
        Object obj = this.getValueN(key);
        @SuppressWarnings("unchecked")
        T t = (T) obj;
        return t;
    }

    private native Object getValueN(String key);

    public void setValue(String key, int value) throws OcException {
        this.setValueInteger(key, value);
    }

    public void setValue(String key, double value) throws OcException {
        this.setValueDouble(key, value);
    }

    public void setValue(String key, boolean value) throws OcException {
        this.setValueBoolean(key, value);
    }

    public void setValue(String key, String value) throws OcException {
        this.setValueStringN(key, value);
    }

    public void setValue(String key, OcRepresentation value) throws OcException {
        this.setValueRepresentation(key, value);
    }

    public void setValue(String key, int[] value) throws OcException {
        this.setValueIntegerArray(key, value);
    }

    public void setValue(String key, int[][] value) throws OcException {
        this.setValueInteger2DArray(key, value);
    }

    public void setValue(String key, int[][][] value) throws OcException {
        this.setValueInteger3DArray(key, value);
    }

    public void setValue(String key, double[] value) throws OcException {
        this.setValueDoubleArray(key, value);
    }

    public void setValue(String key, double[][] value) throws OcException {
        this.setValueDouble2DArray(key, value);
    }

    public void setValue(String key, double[][][] value) throws OcException {
        this.setValueDouble3DArray(key, value);
    }

    public void setValue(String key, boolean[] value) throws OcException {
        this.setValueBooleanArray(key, value);
    }

    public void setValue(String key, boolean[][] value) throws OcException {
        this.setValueBoolean2DArray(key, value);
    }

    public void setValue(String key, boolean[][][] value) throws OcException {
        this.setValueBoolean3DArray(key, value);
    }

    public void setValue(String key, String[] value) throws OcException {
        this.setValueStringArray(key, value);
    }

    public void setValue(String key, String[][] value) throws OcException {
        this.setValueString2DArray(key, value);
    }

    public void setValue(String key, String[][][] value) throws OcException {
        this.setValueString3DArray(key, value);
    }

    public void setValue(String key, OcRepresentation[] value) throws OcException {
        this.setValueRepresentationArray(key, value);
    }

    public void setValue(String key, OcRepresentation[][] value) throws OcException {
        this.setValueRepresentation2DArray(key, value);
    }

    public void setValue(String key, OcRepresentation[][][] value) throws OcException {
        this.setValueRepresentation3DArray(key, value);
    }

    private native void setValueInteger(String key, int value) throws OcException;

    private native void setValueDouble(String key, double value) throws OcException;

    private native void setValueBoolean(String key, boolean value) throws OcException;

    private native void setValueStringN(String key, String value) throws OcException;

    private native void setValueRepresentation(String key, OcRepresentation value) throws OcException;

    private native void setValueIntegerArray(String key, int[] value) throws OcException;

    private native void setValueInteger2DArray(String key, int[][] value) throws OcException;

    private native void setValueInteger3DArray(String key, int[][][] value) throws OcException;

    private native void setValueDoubleArray(String key, double[] value) throws OcException;

    private native void setValueDouble2DArray(String key, double[][] value) throws OcException;

    private native void setValueDouble3DArray(String key, double[][][] value) throws OcException;

    private native void setValueBooleanArray(String key, boolean[] value) throws OcException;

    private native void setValueBoolean2DArray(String key, boolean[][] value) throws OcException;

    private native void setValueBoolean3DArray(String key, boolean[][][] value) throws OcException;

    private native void setValueStringArray(String key, String[] value) throws OcException;

    private native void setValueString2DArray(String key, String[][] value) throws OcException;

    private native void setValueString3DArray(String key, String[][][] value) throws OcException;

    private native void setValueRepresentationArray(String key, OcRepresentation[] value) throws OcException;

    private native void setValueRepresentation2DArray(String key, OcRepresentation[][] value) throws OcException;

    private native void setValueRepresentation3DArray(String key, OcRepresentation[][][] value) throws OcException;

    /**
     * @deprecated use {@link #getValue(String key)} instead.
     */
    @Deprecated
    public int getValueInt(String key) {
        Integer value = 0;
        try {
            value = this.getValue(key);
        } catch (OcException e) {
            //simply catching here for the deprecated APIs, so the older usages don't have to handle
            //it in the code
        }
        return value;
    }

    /**
     * @deprecated use {@link #getValue(String key)} instead.
     */
    @Deprecated
    public boolean getValueBool(String key) {
        Boolean value = false;
        try {
            value = this.getValue(key);
        } catch (OcException e) {
            //simply catching here for the deprecated APIs, so the older usages don't have to handle
            //it in the code
        }
        return value;
    }

    /**
     * @deprecated use {@link #getValue(String key)} instead.
     */
    @Deprecated
    public String getValueString(String key) {
        String value = "";
        try {
            value = this.getValue(key);
        } catch (OcException e) {
            //simply catching here for the deprecated APIs, so the older usages don't have to handle
            //it in the code
        }
        return value;
    }

    /**
     * @deprecated use {@link #setValue(String key, int value)} instead.
     */
    @Deprecated
    public void setValueInt(String key, int value) {
        try {
            this.setValue(key, value);
        } catch (OcException e) {
            //simply catching here for the deprecated APIs, so the older usages don't have to handle
            //it in the code
        }
    }

    /**
     * @deprecated use {@link #setValue(String key, boolean value)} instead.
     */
    @Deprecated
    public void setValueBool(String key, boolean value) {
        try {
            this.setValue(key, value);
        } catch (OcException e) {
            //simply catching here for the deprecated APIs, so the older usages don't have to handle
            //it in the code
        }
    }

    /**
     * @deprecated use {@link #setValue(String key, String value)} instead.
     */
    @Deprecated
    public void setValueString(String key, String value) {
        try {
            this.setValue(key, value);
        } catch (OcException e) {
            //simply catching here for the deprecated APIs, so the older usages don't have to handle
            //it in the code
        }
    }

    public native void addChild(OcRepresentation representation);

    public native void clearChildren();

    public List<OcRepresentation> getChildren() {
        return Arrays.asList(
                getChildrenArray());
    }

    private native OcRepresentation[] getChildrenArray();

    public native String getUri();

    public native void setUri(String uri);

    /**
     * Method to get the list of resource types
     *
     * @return List of resource types
     */
    public native List<String> getResourceTypes();

    /**
     * Method to set the list of resource types
     *
     * @param resourceTypeList list of resources
     */
    public void setResourceTypes(List<String> resourceTypeList) {
        if (null == resourceTypeList) {
            throw new InvalidParameterException("resourceTypeList cannot be null");
        }

        this.setResourceTypeArray(
                resourceTypeList.toArray(
                        new String[resourceTypeList.size()]));
    }

    private native void setResourceTypeArray(String[] resourceTypeArray);

    /**
     * Method to get the list of resource interfaces
     *
     * @return List of resource interface
     */
    public native List<String> getResourceInterfaces();

    /**
     * Method to set the list of resource interfaces
     *
     * @param resourceInterfaceList list of interfaces
     */
    public void setResourceInterfaces(List<String> resourceInterfaceList) {
        if (null == resourceInterfaceList) {
            throw new InvalidParameterException("resourceInterfaceList cannot be null");
        }

        this.setResourceInterfaceArray(
                resourceInterfaceList.toArray(
                        new String[resourceInterfaceList.size()]));
    }

    private native void setResourceInterfaceArray(String[] resourceInterfaceArray);

    public native boolean isEmpty();

    public native int size();

    public native boolean remove(String key);

    public native boolean hasAttribute(String key);

    public native void setNull(String key);

    public native boolean isNull(String key);

    @Override
    protected void finalize() throws Throwable {
        super.finalize();

        dispose(this.mNativeNeedsDelete);
    }

    private native void create();

    private native void dispose(boolean needsDelete);

    private long mNativeHandle;
    private boolean mNativeNeedsDelete;
}