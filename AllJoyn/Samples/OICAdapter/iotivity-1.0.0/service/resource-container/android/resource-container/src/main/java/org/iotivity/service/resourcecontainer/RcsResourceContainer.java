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
 * @file
 * This file contains the Resource Container APIs
 */
package org.iotivity.service.resourcecontainer;

import java.util.List;
import java.util.Map;

// TODO null check for parameters
/**
 * This class provides APIs for managing the container and bundles in the
 * container.
 */
public class RcsResourceContainer {

    static {
        System.loadLibrary("gnustl_shared");
        System.loadLibrary("oc_logger");
        System.loadLibrary("connectivity_abstraction");
        System.loadLibrary("ca-interface");
        System.loadLibrary("octbstack");
        System.loadLibrary("oc");
        System.loadLibrary("rcs_client");
        System.loadLibrary("rcs_server");
        System.loadLibrary("rcs_common");
        System.loadLibrary("rcs_container");
        System.loadLibrary("resource_container_jni");
    }

    private static RcsResourceContainer sInstance = new RcsResourceContainer();

    private native void nativeStartContainer(String configFile);

    private native void nativeStopContainer();

    private native void nativeAddBundle(String bundleId, String bundleUri,
            String bundlePath, String activator, Map<String, String> params);

    private native void nativeRemoveBundle(String bundleId);

    private native List<RcsBundleInfo> nativeListBundles();

    private native void nativeStartBundle(String bundleId);

    private native void nativeStopBundle(String bundleId);

    private native void nativeAddResourceConfig(String bundleId,
            String resourceUri, Map<String, String> params);

    private native void nativeRemoveResourceConfig(String bundleId,
            String resourceUri);

    private native List<String> nativeListBundleResources(String bundleId);

    /**
     * API for getting the Instance of ResourceContainer class
     *
     */
    public static RcsResourceContainer getInstance() {
        return sInstance;
    }

    /**
     * API for starting the Container
     *
     * <p>
     * This API start the container with the provided Configuration file.
     *
     * @param configFile
     *            configuration File that contains the Bundle/Bundles
     *            information.
     *
     */
    public void startContainer(String configFile) {
        nativeStartContainer(configFile);
    }

    /**
     * API for stopping the Container
     */
    public void stopContainer() {
        nativeStopContainer();
    }

    /**
     * API for getting the list of all bundles in the container
     *
     * @return list<RCSBundleInfo> -List of BundleInfo objects each associated
     *         with a bundle
     *
     *         {@link RcsBundleInfo}
     */
    public List<RcsBundleInfo> listBundles() {
        return nativeListBundles();
    }

    /**
     * API for adding the bundle to the Container
     *
     * @param bundleId
     *            Id of the Bundle
     * @param bundleUri
     *            Uri of the bundle
     * @param bundlePath
     *            Path of the bundle
     * @param activator
     *            Activation prefix for .so bundles, or activator class name for
     *            .jar bundles
     * @param params
     *            key-value pairs in string form for other Bundle parameters
     *
     *            <p>
     *            It is dynamic configuration
     */
    public void addBundle(String bundleId, String bundleUri, String bundlePath,
            String activator, Map<String, String> params) {
        nativeAddBundle(bundleId, bundleUri, bundlePath, activator, params);
    }

    /**
     * API for removing the bundle from the container
     *
     * @param bundleId
     *            Id of the Bundle
     *
     */
    public void removeBundle(String bundleId) {
        nativeRemoveBundle(bundleId);
    }

    /**
     * API for starting the bundle.
     *
     * @param bundleId
     *            Id of the Bundle
     *
     */
    public void startBundle(String bundleId) {
        nativeStartBundle(bundleId);
    }

    /**
     * API for Stopping the bundle
     *
     * @param bundleId
     *            Id of the Bundle
     *
     */
    public void stopBundle(String bundleId) {
        nativeStopBundle(bundleId);
    }

    /**
     * API for adding the Resource configuration information to the bundle
     *
     * @param bundleId
     *            Id of the Bundle
     * @param resourceUri
     *            URI of the resource
     * @param params
     *            key-value pairs in string form for other Bundle parameters
     *
     */
    public void addResourceConfig(String bundleId, String resourceUri,
            Map<String, String> params) {
        nativeAddResourceConfig(bundleId, resourceUri, params);
    }

    /**
     * API for removing the Resource configuration information from the bundle
     *
     * @param bundleId
     *            Id of the Bundle
     * @param resourceUri
     *            URI of the resource
     *
     */
    public void removeResourceConfig(String bundleId, String resourceUri) {
        nativeRemoveResourceConfig(bundleId, resourceUri);
    }

    /**
     * API for getting the list of Bundle Resources
     *
     * @param bundleId
     *            Id of the Bundle
     *
     * @return List<String> All the bundle resources
     */
    public List<String> listBundleResources(String bundleId) {
        return nativeListBundleResources(bundleId);
    }
}
