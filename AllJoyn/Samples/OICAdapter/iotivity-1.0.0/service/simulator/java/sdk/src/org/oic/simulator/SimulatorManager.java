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
 * This class provides a set of methods for creation, discovery and deletion of
 * resources.
 */
public class SimulatorManager {

    /**
     * API for creating a resource from a RAML configuration file whose path is
     * given as a parameter.
     *
     * @param configPath
     *            Path to RAML configuration file.
     * @param listener
     *            Listener for receiving notifications whenever there is a
     *            change in the resource model.
     *
     * @return {@link SimulatorResourceServer} - Created resource on success,
     *         otherwise null.
     *
     * @throws InvalidArgsException
     *             Thrown if the input parameters are empty.
     * @throws SimulatorException
     *             Thrown for other errors.
     */
    public static SimulatorResourceServer createResource(String configPath,
            IResourceModelChangedListener listener)
            throws InvalidArgsException, SimulatorException {
        if (configPath.isEmpty() || null == listener)
            throw new InvalidArgsException(
                    SimulatorResult.SIMULATOR_INVALID_PARAM.ordinal(),
                    "Parameter passed in invalid");
        SimulatorResourceServer simulatorResourceServerObj;
        simulatorResourceServerObj = SimulatorManagerNativeInterface
                .createResource(configPath, listener);
        return simulatorResourceServerObj;
    }

    /**
     * API for creating a set of resources from a RAML configuration file whose
     * path is given as a parameter.
     *
     * @param configPath
     *            Path to RAML configuration file.
     * @param count
     *            Number of resources to be created.
     * @param listener
     *            Listener for receiving notifications whenever there is a
     *            change in the resource model.
     *
     * @return Returns an array of {@link SimulatorResourceServer} objects one
     *         for each created resource on success, otherwise null.
     *
     * @throws InvalidArgsException
     *             Thrown if the input parameters are empty.
     * @throws SimulatorException
     *             Thrown for other errors.
     */
    public static SimulatorResourceServer[] createResource(String configPath,
            int count, IResourceModelChangedListener listener)
            throws InvalidArgsException, SimulatorException {
        if (configPath.isEmpty() || count < 0 || null == listener)
            throw new InvalidArgsException(
                    SimulatorResult.SIMULATOR_INVALID_PARAM.ordinal(),
                    "Parameter passed in invalid");
        SimulatorResourceServer[] simulatorResourceServers;
        simulatorResourceServers = SimulatorManagerNativeInterface
                .createResources(configPath, count, listener);
        return simulatorResourceServers;
    }

    /**
     * API for deleting a specific resource.
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
    public static void deleteResource(SimulatorResourceServer resource)
            throws InvalidArgsException, SimulatorException {
        SimulatorManagerNativeInterface.deleteResource(resource);
    }

    /**
     * API for deleting either all the resources or resources of a specific
     * type. Ex: If resourceType is oic.light, all resources of oic.light type
     * will be deleted. If resourceType is null, then all of the resources will
     * be deleted.
     *
     * @param resourceType
     *            Type of resource to be deleted.
     *
     * @throws SimulatorException
     *             Thrown for other errors.
     */
    public static void deleteResources(String resourceType)
            throws SimulatorException {
        SimulatorManagerNativeInterface.deleteResources(resourceType);
    }

    /**
     * API for discovering all types of resources in the network. Callback is
     * called when a resource is discovered in the network.
     *
     * @param listener
     *            Interface to receive the discovered remote resources.
     *
     * @throws InvalidArgsException
     *             Thrown if the input parameter is empty.
     * @throws SimulatorException
     *             Thrown for other errors.
     */
    public static void findResource(IFindResourceListener listener)
            throws InvalidArgsException, SimulatorException {
        SimulatorManagerNativeInterface.findResource(null, listener);
    }

    /**
     * API for discovering specific type of resources in the network. Callback
     * is called when a resource is discovered in the network.
     *
     * @param resourceType
     *            Required resource type
     * @param listener
     *            Interface to receive the discovered remote resources.
     *
     * @throws InvalidArgsException
     *             Thrown if the input parameter is empty.
     * @throws SimulatorException
     *             Thrown for other errors.
     */
    public static void findResource(String resourceType,
            IFindResourceListener listener) throws InvalidArgsException,
            SimulatorException {
        if (null == resourceType || resourceType.isEmpty()) {
            throw new InvalidArgsException(
                    SimulatorResult.SIMULATOR_INVALID_PARAM.ordinal(),
                    "Resource type is empty");
        }
        SimulatorManagerNativeInterface.findResource(resourceType, listener);
    }

    /**
     * API to set the listener for receiving log messages.
     *
     * @param logger
     *            {@link ILogger} to receive the log messages.
     */
    public static void setLogger(ILogger logger) {
        SimulatorManagerNativeInterface.setLogger(logger);
    }

    /**
     * API to set the device information.
     *
     * @param deviceInfo
     *            Device information.
     */
    public static void setDeviceInfo(String deviceInfo) {
        SimulatorManagerNativeInterface.setDeviceInfo(deviceInfo);
    }

    /**
     * API to get the device information asynchronously via callback
     * using {@link IDeviceInfo}.
     *
     * @param listener
     *            Interface for receiving the device information.
     */
    public static void getDeviceInfo(IDeviceInfo listener) {
        SimulatorManagerNativeInterface.getDeviceInfo(listener);
    }

    /**
     * API to set the platform information.
     *
     * @param platformInfo
     *            {@link PlatformInfo} - Platform information.
     */
    public static void setPlatformInfo(PlatformInfo platformInfo) {
        SimulatorManagerNativeInterface.setPlatformInfo(platformInfo);
    }

    /**
     * API to get the platform information asynchronously via callback
     * using {@link IPlatformInfo}..
     *
     * @param listener
     *            Interface for receiving the platform information.
     */
    public static void getPlatformInfo(IPlatformInfo listener) {
        SimulatorManagerNativeInterface.getPlatformInfo(listener);
    }
}