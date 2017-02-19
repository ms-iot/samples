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
import java.util.Vector;

/**
 * The BaseActivator implements the native interface to the resource container.
 * It loads the resource container library and provies native methods that can
 * be used to register and unregister resources.
 */
public class BaseActivator implements BundleActivator {
	private String bundleId;
	private Vector<BundleResource> bundleResources = new Vector<BundleResource>();

	/**
	 * Creates an instance of the BaseActivator
	 * 
	 * @param bundleId
	 *            unique bundle identifier (e.g., oic.bundle.hue)
	 */
	public BaseActivator(String bundleId) {
		this.bundleId = bundleId;
	}

	static {
		try {
			System.loadLibrary("rcs_container");
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	/**
	 * Bundle activation needs to be provided by the subclass.
	 */
	public void activateBundle() {

	}

	/**
	 * Deactivates the bundle and unregisters its resources.
	 */
	public void deactivateBundle() {
		System.out.println("Deactivating bundle (Base Activator).");
		for (BundleResource bundleResource : bundleResources) {
			unregisterResource(bundleResource);
		}
	}

	/**
	 * Registers a bundle resource at the resource container.
	 * 
	 * @param resource
	 *            bundle resource instance that should be made available as OIC
	 *            resource
	 */
	public void registerResource(BundleResource resource) {
		bundleResources.add(resource);
		registerJavaResource(resource, resource.getAttributeKeys(), bundleId,
				resource.getURI(), resource.getResourceType(),
				resource.getName());
	}

	/**
	 * Wrapper to retrieve the resource configuration of the bundle resources.
	 * 
	 * @return List of resource configurations.
	 */
	public List<ResourceConfig> getConfiguredBundleResources() {
		int configuredResources = getNumberOfConfiguredResources(bundleId);

		Vector<ResourceConfig> configs = new Vector<ResourceConfig>();

		for (int i = 0; i < configuredResources; i++) {
			String[] resourceParams = getConfiguredResourceParams(bundleId, i);
			ResourceConfig config = new ResourceConfig(resourceParams);
			configs.add(config);

		}
		return configs;
	}

	/**
	 * Unregisters a resource from the resource container.
	 */
	public void unregisterResource(BundleResource resource) {
		bundleResources.remove(resource);
		unregisterJavaResource(resource, resource.getURI());
	}

	/**
	 * Native method that calls to the resource container.
	 * 
	 * @param attributes
	 *            String array of attribute names
	 * @param bundleId
	 *            unique bundle identifier
	 * @param uri
	 *            Uri that should be used to register the resource
	 */
	private native void registerJavaResource(BundleResource resource,
			String[] attributes, String bundleId, String uri,
			String resourceType, String name);

	/**
	 * Native method that calls to the resource container.
	 * 
	 * @param resource
	 * @param uri
	 */
	private native void unregisterJavaResource(BundleResource resource,
			String uri);

	/**
	 * Native method to retrieve the number of configured resources.
	 * 
	 * @param bundleId
	 *            unique bundle identifier
	 */
	private native int getNumberOfConfiguredResources(String bundleId);

	/**
	 * Native method to retrieve the configured resource parameters.
	 * 
	 * @param bundleId
	 *            unique bundle identifier
	 * @param resId
	 *            get the resource params for a certain resource
	 */
	private native String[] getConfiguredResourceParams(String bundleId,
			int resId);

}
