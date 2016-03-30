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

import java.util.HashMap;
import java.util.Set;

/**
 * Basic BundleResource that should be used as a base class by a bundle
 * resources. A concrete technology has to override the setAttribute and
 * getAttribute method and map the according reads and writes to the technology
 * specific messages.
 */
public abstract class BundleResource {
	protected String m_name, m_uri, m_resourceType, m_address;

	protected HashMap<String, String> m_attributes = new HashMap<String, String>();

	/**
	 * Initialize the internal attribute structure.
	 */
	protected abstract void initAttributes();

	/**
	 * Set the attribute (map to a send command for the according protocol)
	 * 
	 * @param key
	 *            name of the attribute to be set
	 * @param value
	 *            new value of the attribute
	 */
	protected final void setAttribute(String key, String value) {
		m_attributes.put(key, value);
	}

	/**
	 * Set the attribute (map to a send command for the according protocol)
	 * 
	 * @param key
	 *            name of the attribute to be set
	 * @param value
	 *            new value of the attribute
	 */
	public abstract void handleSetAttributeRequest(String key, String value);

	/**
	 * Retrieve the attribute (only data)
	 * 
	 * @param key
	 *            name of the attribute to be read
	 * @return Value of the attribute
	 */
	protected final String getAttribute(String key) {
		return m_attributes.get(key);
	}

	/**
	 * Retrieve the attribute (map to read command)
	 * 
	 * @param key
	 *            name of the attribute to be set
	 * @param value
	 *            new value of the attribute
	 */
	public abstract String handleGetAttributeRequest(String key);

	/**
	 * Attribute keys provided through by the bundle resource.
	 * 
	 * @return Name of attribute keys as string array
	 */
	public String[] getAttributeKeys() {
		Set<String> keys = m_attributes.keySet();
		return keys.toArray(new String[keys.size()]);
	}

	/**
	 * Setter for the uri property
	 * 
	 * @param uri
	 *            URI of the resource
	 */
	public void setURI(String uri) {
		this.m_uri = uri;
	}

	/**
	 * Returns the URI of the resource
	 * 
	 * @return Resource URI
	 */
	public String getURI() {
		return m_uri;
	}

	/**
	 * Sets the resource type property
	 * 
	 * @param resourceType
	 *            OIC resource type
	 */
	public void setResourceType(String resourceType) {
		this.m_resourceType = resourceType;
	}

	/**
	 * Getter for the resource type
	 * 
	 * @return OIC resource type
	 */
	public String getResourceType() {
		return m_resourceType;
	}

	/**
	 * Sets the technology specific address information (e.g., ZigBee short or
	 * long identifier)
	 * 
	 * @param address
	 *            Resource address
	 */
	public void setAddress(String address) {
		this.m_address = address;
	}

	/**
	 * Returns the technology specific address information
	 * 
	 * @return Resource address
	 */
	public String getAddress() {
		return m_address;
	}

	/**
	 * Sets the name property of the resource
	 * 
	 * @param name
	 *            Resource name
	 */
	public void setName(String name) {
		this.m_name = name;
	}

	/**
	 * Returns the name property of the resource
	 * 
	 * @return Resource name
	 */
	public String getName() {
		return m_name;
	}

}
