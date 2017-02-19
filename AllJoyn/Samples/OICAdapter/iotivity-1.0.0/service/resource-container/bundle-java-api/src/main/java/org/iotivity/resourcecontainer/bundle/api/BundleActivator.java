//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

package org.iotivity.resourcecontainer.bundle.api;

import java.util.List;

/**
 * The BundleActivator interface needs to be implemented. A bundle provider
 * can directly extend fromt the BaseActivator.
 */
public interface BundleActivator {
    /**
     * Activates the bundle and creates all resources.
     */
    public void activateBundle();

    /**
     * Deactivates the bundle and destroys all resources.
     */
    public void deactivateBundle();

    /**
     * Registers a single resource instance at the resource container
     * @param resource Instance of a BundleResource
     */
    public void registerResource(BundleResource resource);

    /**
     * Unregisters a single resource instance at the resource container
     * @param resource Instance of a BundleResource
     */
    public void unregisterResource(BundleResource resource);

    /**
     * List the configuration of the bundle resources.
     * @return List of configuration for each resource 
     */
    public List<ResourceConfig> getConfiguredBundleResources();
}
