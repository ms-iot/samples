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

import org.oic.simulator.clientcontroller.IFindResourceListener;
import org.oic.simulator.serviceprovider.IResourceModelChangedListener;
import org.oic.simulator.serviceprovider.SimulatorResourceServer;

/**
 * This class provides a set of native functions for creation, discovery and
 * deletion of resources.
 */
class SimulatorManagerNativeInterface {

    /**
     * Native function for creating a resource.
     *
     * @param configPath
     *            Path to RAML configuration file.
     * @param listener
     *            Listener for receiving notifications whenever there is a
     *            change in the resource model.
     *
     * @return {@link SimulatorResourceServer} object on success, otherwise
     *         null.
     *
     * @throws InvalidArgsException
     *             Thrown if the input parameters are empty.
     * @throws SimulatorException
     *             Thrown for other errors.
     */
    public static native SimulatorResourceServer createResource(
            String configPath, IResourceModelChangedListener listener)
            throws InvalidArgsException, SimulatorException;

    /**
     * Native function for creating several resources.
     *
     * @param configPath
     *            Path to RAML configuration file.
     * @param count
     *            Number of instances.
     * @param listener
     *            Listener for receiving notifications whenever there is a
     *            change in the resource model.
     *
     * @return An array of {@link SimulatorResourceServer} objects on success,
     *         otherwise null.
     *
     * @throws InvalidArgsException
     *             Thrown if the input parameters are empty.
     * @throws SimulatorException
     *             Thrown for other errors.
     */
    public static native SimulatorResourceServer[] createResources(
            String configPath, int count, IResourceModelChangedListener listener)
            throws InvalidArgsException, SimulatorException;

    /**
     * Native function to delete a specific resource.
     *
     * @param resource
     *            {@link SimulatorResourceServer} object of the resource to be
     *            deleted.
     *
     * @throws InvalidArgsException
     *             Thrown if the input parameter is empty.
     * @throws SimulatorException
     *             Thrown for other errors.
     */
    public static native void deleteResource(SimulatorResourceServer resource)
            throws InvalidArgsException, SimulatorException;

    /**
     * Native function to delete all resources or resources of a specific type.
     *
     * @param resourceType
     *            Type of the resource.
     *
     * @throws InvalidArgsException
     *             Thrown if the input parameter is empty.
     * @throws SimulatorException
     *             Thrown for other errors.
     */
    public static native void deleteResources(String resourceType)
            throws SimulatorException;

    /**
     * Native function for discovering resources.
     *
     * @param resourceType
     *            required resource type
     * @param listener
     *            Interface to receive the discovered remote resources.
     *
     * @throws InvalidArgsException
     *             Thrown if the input parameter is empty.
     * @throws SimulatorException
     *             Thrown for other errors.
     */
    public static native void findResource(String resourceType,
            IFindResourceListener listener) throws InvalidArgsException,
            SimulatorException;

    /**
     * Native function to set the logger listener for receiving the log messages
     * from native layer.
     *
     * @param logger
     *            Interface to receive log.
     */
    public static native void setLogger(ILogger logger);

    /**
     * Native function to set the device information.
     *
     * @param deviceInfo
     *            Device information.
     */
    public static native void setDeviceInfo(String deviceInfo);

    /**
     * Native function to get the device information asynchronously via the
     * listener.
     *
     * @param listener
     *            Interface for receiving the device information.
     */
    public static native void getDeviceInfo(IDeviceInfo listener);

    /**
     * Native function to set the platform information.
     *
     * @param platformInfo
     *            Platform information.
     */
    public static native void setPlatformInfo(PlatformInfo platformInfo);

    /**
     * Native function to get the platform information asynchronously via the
     * listener.
     *
     * @param listener
     *            Interface for receiving the platform information.
     */
    public static native void getPlatformInfo(IPlatformInfo listener);
}