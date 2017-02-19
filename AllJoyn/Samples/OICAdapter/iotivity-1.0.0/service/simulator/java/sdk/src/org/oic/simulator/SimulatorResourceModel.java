/*
 * Copyright 2015 Samsung Electronics All Rights Reserved.
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
 */

package org.oic.simulator;

import org.oic.simulator.ResourceAttribute;
import java.util.Map;

/**
 * This class represents the resource model of a resource and it provides a set
 * of native methods for accessing the resource model.
 */
public class SimulatorResourceModel {

    /**
     * Constructor for creating a native resource model object. Client requests
     * such as PUT and POST uses this method for passing the new/updated
     * resource model.
     */
    public SimulatorResourceModel() {
        create();
    }

    /**
     * API to add an attribute of type integer.
     *
     * @param name
     *            Name of the attribute
     * @param value
     *            Value of the attribute
     *
     * @throws InvalidArgsException
     *             This exception will be thrown if the attribute name is
     *             invalid.
     * @throws SimulatorException
     *             This exception will be thrown either if the resource model is
     *             not found or for some general errors.
     */
    public native void addAttributeInt(String name, int value)
            throws InvalidArgsException, SimulatorException;

    /**
     * API to add an attribute of type double.
     *
     * @param name
     *            Name of the attribute
     * @param value
     *            Value of the attribute
     *
     * @throws InvalidArgsException
     *             This exception will be thrown if the attribute name is
     *             invalid.
     * @throws SimulatorException
     *             This exception will be thrown either if the resource model is
     *             not found or for some general errors.
     */
    public native void addAttributeDouble(String name, double value)
            throws InvalidArgsException, SimulatorException;

    /**
     * API to add an attribute of type boolean.
     *
     * @param name
     *            Name of the attribute
     * @param value
     *            Value of the attribute
     *
     * @throws InvalidArgsException
     *             This exception will be thrown if the attribute name is
     *             invalid.
     * @throws SimulatorException
     *             This exception will be thrown either if the resource model is
     *             not found or for some general errors.
     */
    public native void addAttributeBoolean(String name, boolean value)
            throws InvalidArgsException, SimulatorException;

    /**
     * API to add an attribute of type string.
     *
     * @param name
     *            Name of the attribute
     * @param value
     *            Value of the attribute
     *
     * @throws InvalidArgsException
     *             This exception will be thrown if the attribute name is
     *             invalid.
     * @throws SimulatorException
     *             This exception will be thrown either if the resource model is
     *             not found or for some general errors.
     */
    public native void addAttributeString(String name, String value)
            throws InvalidArgsException, SimulatorException;

    /**
     * API to get number of attributes for this model.
     *
     * @return Number of attributes.
     *
     * @throws SimulatorException
     *             This exception will be thrown either if the resource model is
     *             not found or for some general errors.
     */
    public native int size() throws SimulatorException;

    /**
     * API for getting all attributes.
     *
     * @return Map of attributes with attribute name as the key and its
     *         corresponding {@link ResourceAttribute} object as the value.
     *
     * @throws SimulatorException
     *             This exception will be thrown either if the resource model is
     *             not found or for some general errors.
     */
    public native Map<String, ResourceAttribute> getAttributes()
            throws SimulatorException;

    /**
     * API to get attribute by its name.
     *
     * @param name
     *            Name of the attribute
     *
     * @return An object of {@link ResourceAttribute}.
     *
     * @throws InvalidArgsException
     *             This exception will be thrown if the attribute does not
     *             exist.
     *
     * @throws SimulatorException
     *             This exception will be thrown either if the resource model is
     *             not found or for some general errors.
     */
    public native ResourceAttribute getAttribute(String name)
            throws InvalidArgsException, SimulatorException;

    private SimulatorResourceModel(long nativeHandle) {
        this.nativeHandle = nativeHandle;
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            dispose();
        } catch(Throwable t){
            throw t;
        } finally{
            super.finalize();
        }
    }

    private native void create();

    private native void dispose();

    private long nativeHandle;
}