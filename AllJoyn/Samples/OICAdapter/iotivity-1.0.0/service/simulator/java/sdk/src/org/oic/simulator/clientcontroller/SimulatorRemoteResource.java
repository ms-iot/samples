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

package org.oic.simulator.clientcontroller;

import java.util.LinkedList;
import java.util.Map;

import org.oic.simulator.InvalidArgsException;
import org.oic.simulator.NoSupportException;
import org.oic.simulator.OperationInProgressException;
import org.oic.simulator.SimulatorException;
import org.oic.simulator.SimulatorResourceModel;
import org.oic.simulator.SimulatorResult;

/**
 * SimulatorRemoteResource represents a Resource running in the remote Simulator
 * Server. It comes with a well-defined contract or interface onto which you can
 * perform different operations or subscribe for event notifications.
 */
public class SimulatorRemoteResource {

    private SimulatorRemoteResource(long nativeHandle) {
        this.nativeHandle = nativeHandle;
    }

    /**
     * API to get the URI for this resource.
     *
     * @return Resource URI
     */
    public String getUri() {
        return mUri;
    }

    /**
     * API to get the observe policy of this resource.
     *
     * @return True if the resource is observable, otherwise false.
     */
    public boolean getIsObservable() {
        return mIsObservable;
    }

    /**
     * API to get the connectivity type for this resource.
     *
     * @return Connectivity type.
     */
    public SimulatorConnectivityType getConnectivityType() {
        return SimulatorConnectivityType.getConnectivityType(mConnType);
    }

    /**
     * API to get the list of resource types.
     *
     * @return List of resource types.
     */
    public LinkedList<String> getResourceTypes() {
        return mResTypes;
    }

    /**
     * API to get the list of resource interfaces.
     *
     * @return List of resource interfaces.
     */
    public LinkedList<String> getResourceInterfaces() {
        return mResInterfaces;
    }

    /**
     * API to get host address and port information of the resource.
     *
     * @return Host address.
     */
    public String getHost() {
        return mHost;
    }

    /**
     * API to get a unique Id of the resource .
     *
     * @return Unique ID.
     */
    public String getId() {
        return mId;
    }

    /**
     * API to start observing the resource.
     *
     * @param observeType
     *            Allows the client to specify how it wants to observe.
     * @param queryParamsMap
     *            Map which can have the query parameter names and values.
     * @param onObserveListener
     *            The handler method which will be invoked with a map of
     *            attribute names and values whenever there is a change in
     *            resource model of the remote resource.
     *
     * @throws InvalidArgsException
     *             This exception will be thrown if any parameter has invalid
     *             values.
     * @throws SimulatorException
     *             This exception will be thrown for other errors.
     */
    public native void startObserve(SimulatorObserveType observeType,
            Map<String, String> queryParamsMap,
            IObserveListener onObserveListener) throws InvalidArgsException,
            SimulatorException;

    /**
     * API to stop observing the resource.
     *
     * @throws InvalidArgsException
     *             This exception will be thrown if the native remote resource
     *             object is unavailable.
     * @throws SimulatorException
     *             This exception will be thrown for other errors.
     */
    public native void stopObserve() throws InvalidArgsException,
            SimulatorException;

    /**
     * API to send GET request to the resource. Response will be notified
     * asynchronously via callback set for {@link IGetListener}.
     *
     * @param queryParamsMap
     *            Map which can have the query parameter name and value.
     * @param onGetListener
     *            Event handler which will be invoked with the response for GET
     *            request with a map of attribute name and values.
     *
     * @throws InvalidArgsException
     *             This exception will be thrown if any parameter has invalid
     *             values.
     * @throws NoSupportException
     *             This exception will be thrown if we cannot send GET request
     *             to the remote resource.
     * @throws SimulatorException
     *             This exception will be thrown for other errors.
     */
    public void get(Map<String, String> queryParamsMap,
            IGetListener onGetListener) throws InvalidArgsException,
            NoSupportException, SimulatorException {
        if (null == onGetListener)
            throw new InvalidArgsException(
                    SimulatorResult.SIMULATOR_INVALID_PARAM.ordinal(),
                    "Parameter passed in invalid");
        this.nativeGet(null, queryParamsMap, onGetListener);
    }

    /**
     * API to send GET request to the resource. Response will be notified
     * asynchronously via callback set for {@link IGetListener}.
     *
     * @param resourceInterface
     *            Interface type of the resource to operate on.
     * @param queryParamsMap
     *            Map which can have the query parameter name and value.
     * @param onGetListener
     *            Event handler which will be invoked with the response for GET
     *            request with a map of attribute name and values.
     *
     * @throws InvalidArgsException
     *             This exception will be thrown if any parameter has invalid
     *             values.
     * @throws NoSupportException
     *             This exception will be thrown if we cannot send GET request
     *             to the remote resource.
     * @throws SimulatorException
     *             This exception will be thrown for other errors.
     */
    public void get(String resourceInterface,
            Map<String, String> queryParamsMap, IGetListener onGetListener)
            throws InvalidArgsException, NoSupportException, SimulatorException {
        if (null == resourceInterface || resourceInterface.isEmpty() || null == onGetListener)
            throw new InvalidArgsException(
                    SimulatorResult.SIMULATOR_INVALID_PARAM.ordinal(),
                    "Parameter passed in invalid");
        this.nativeGet(resourceInterface, queryParamsMap, onGetListener);
    }

    private native void nativeGet(String resourceInterface,
            Map<String, String> queryParamsMap, IGetListener onGetListener)
            throws InvalidArgsException, NoSupportException, SimulatorException;

    /**
     * API to send PUT request to the resource. Response will be notified
     * asynchronously via callback set for {@link IPutListener}.
     *
     * @param representation
     *            {@link SimulatorResourceModel} holding the representation of
     *            the resource.
     * @param queryParamsMap
     *            Map which can have the query parameter name and value.
     * @param onPutListener
     *            Event handler which will be invoked with the response for PUT
     *            request with a map of attribute name and values.
     *
     * @throws InvalidArgsException
     *             This exception will be thrown if any parameter has invalid
     *             value.
     * @throws NoSupportException
     *             This exception will be thrown if we cannot send PUT request
     *             to the remote resource.
     * @throws SimulatorException
     *             This exception will be thrown for other errors.
     */
    public void put(SimulatorResourceModel representation,
            Map<String, String> queryParamsMap, IPutListener onPutListener)
            throws InvalidArgsException, NoSupportException, SimulatorException {
        if (null == representation || null == onPutListener)
            throw new InvalidArgsException(
                    SimulatorResult.SIMULATOR_INVALID_PARAM.ordinal(),
                    "Parameter passed in invalid");
        this.nativePut(null, representation, queryParamsMap, onPutListener);
    }

    /**
     * API to send PUT request to the resource. Response will be notified
     * asynchronously via callback set for {@link IPutListener}.
     *
     * @param resourceInterface
     *            Interface type of the resource to operate on.
     * @param representation
     *            {@link SimulatorResourceModel} holding the representation of
     *            the resource.
     * @param queryParamsMap
     *            Map which can have the query parameter name and value.
     * @param onPutListener
     *            Event handler which will be invoked with the response for PUT
     *            request with a map of attribute name and values.
     *
     * @throws InvalidArgsException
     *             This exception will be thrown if any parameter has invalid
     *             value.
     * @throws NoSupportException
     *             This exception will be thrown if we cannot send PUT request
     *             to the remote resource.
     * @throws SimulatorException
     *             This exception will be thrown for other errors.
     */
    public void put(String resourceInterface,
            SimulatorResourceModel representation,
            Map<String, String> queryParamsMap, IPutListener onPutListener)
            throws InvalidArgsException, NoSupportException, SimulatorException {
        if (null == resourceInterface || resourceInterface.isEmpty() ||
            null == representation || null == onPutListener)
            throw new InvalidArgsException(
                    SimulatorResult.SIMULATOR_INVALID_PARAM.ordinal(),
                    "Parameter passed in invalid");
        this.nativePut(resourceInterface, representation, queryParamsMap, onPutListener);
    }

    private native void nativePut(String resourceInterface,
            SimulatorResourceModel representation,
            Map<String, String> queryParamsMap, IPutListener onPutListener)
            throws InvalidArgsException, NoSupportException, SimulatorException;

    /**
     * API to send POST request to the resource. Response will be notified
     * asynchronously via callback set for {@link IPostListener}.
     *
     * @param representation
     *            {@link SimulatorResourceModel} holding the representation of
     *            the resource
     * @param queryParamsMap
     *            Map which can have the query parameter name and value
     * @param onPostListener
     *            Event handler which will be invoked with the response for POST
     *            request with a map of attribute name and values.
     *
     * @throws InvalidArgsException
     *             This exception will be thrown if any parameter has invalid
     *             value.
     * @throws NoSupportException
     *             This exception will be thrown if we cannot send POST request
     *             on the remote resource.
     * @throws SimulatorException
     *             This exception will be thrown for other errors.
     */
    public void post(SimulatorResourceModel representation,
            Map<String, String> queryParamsMap, IPostListener onPostListener)
            throws InvalidArgsException, NoSupportException, SimulatorException {
        if (null == representation || null == onPostListener)
            throw new InvalidArgsException(
                    SimulatorResult.SIMULATOR_INVALID_PARAM.ordinal(),
                    "Parameter passed in invalid");
        this.nativePost(null, representation, queryParamsMap, onPostListener);
    }

    /**
     * API to send POST request to the resource. Response will be notified
     * asynchronously via callback set for {@link IPostListener}.
     *
     * @param resourceInterface
     *            Interface type of the resource to operate on.
     * @param representation
     *            {@link SimulatorResourceModel} holding the representation of
     *            the resource.
     * @param queryParamsMap
     *            Map which can have the query parameter name and value.
     * @param onPostListener
     *            Event handler which will be invoked with the response for POST
     *            request with a map of attribute name and values.
     *
     * @throws InvalidArgsException
     *             This exception will be thrown if any parameter has invalid
     *             value.
     * @throws NoSupportException
     *             This exception will be thrown if we cannot send POST request
     *             on the remote resource.
     * @throws SimulatorException
     *             This exception will be thrown for other errors.
     */
    public void post(String resourceInterface,
            SimulatorResourceModel representation,
            Map<String, String> queryParamsMap, IPostListener onPostListener)
            throws InvalidArgsException, NoSupportException, SimulatorException {
        if (null == resourceInterface || resourceInterface.isEmpty() ||
            null == representation || null == onPostListener)
            throw new InvalidArgsException(
                SimulatorResult.SIMULATOR_INVALID_PARAM.ordinal(),
                "Parameter passed in invalid");
        this.nativePost(resourceInterface, representation, queryParamsMap, onPostListener);
    }

    private native void nativePost(String resourceInterface,
            SimulatorResourceModel representation,
            Map<String, String> queryParamsMap, IPostListener onPostListener)
            throws InvalidArgsException, NoSupportException, SimulatorException;

    /**
     * API to provide remote resource configure information,
     * which is required for using automation feature.
     *
     * @param path
     *            Path to RAML file.
     *
     * @throws InvalidArgsException
     *             Thrown if the RAML configuration file path is invalid.
     * @throws SimulatorException
     *             Thrown for other errors.
     */
    public native void setConfigInfo(String path)
            throws InvalidArgsException, SimulatorException;

    /**
     * API to send multiple requests for the resource, based on
     * the configure file provided from {@link setConfigInfo}.
     * This verifies response received as well.
     *
     * @param requestType
     *            Request type to verify.
     * @param onVerifyListener
     *            This event handler will be invoked with the current status of
     *            the automation.
     *
     * @return Automation ID.
     *
     * @throws InvalidArgsException
     *             This exception will be thrown if any parameter has invalid
     *             value.
     * @throws NoSupportException
     *             Thrown either if the resource does not support the request
     *             type or the resource is not configured with RAML.
     * @throws OperationInProgressException
     *             Thrown if another request generation session is already in
     *             progress.
     * @throws SimulatorException
     *             This exception will be thrown for other errors.
     */
    public int startVerification(SimulatorVerificationType requestType,
            IVerificationListener onVerifyListener)
            throws InvalidArgsException, NoSupportException,
            OperationInProgressException, SimulatorException {
        return startVerification(requestType.ordinal(), onVerifyListener);
    }

    /**
     * API to stop sending requests which has been started using {@link setConfigInfo}.
     *
     * @param id
     *            Automation ID.
     *
     * @throws InvalidArgsException
     *             Thrown if the automation ID is invalid.
     * @throws NoSupportException
     *             Thrown if the resource is not configured with RAML.
     * @throws SimulatorException
     *             Thrown for other errors.
     */
    public native void stopVerification(int id) throws InvalidArgsException,
            NoSupportException, SimulatorException;

    private native int startVerification(int requestType,
            IVerificationListener onVerifyListener)
            throws InvalidArgsException, NoSupportException,
            OperationInProgressException, SimulatorException;

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

    private native void dispose();

    private long               nativeHandle;
    private String             mUri;
    private int                mConnType;
    private String             mHost;
    private String             mId;
    private LinkedList<String> mResTypes;
    private LinkedList<String> mResInterfaces;
    private boolean            mIsObservable;
}
