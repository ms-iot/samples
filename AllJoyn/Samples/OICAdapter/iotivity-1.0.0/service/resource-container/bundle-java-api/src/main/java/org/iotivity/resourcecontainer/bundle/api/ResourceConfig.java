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

/**
 * This class holds the configuration parameters for a single resource instance provided
 * by the resource bundle.
 */
public class ResourceConfig {
    private String m_name, m_uri, m_resourceType, m_address;

    /**
     * Empty constructor for resoure config.
     */
    public ResourceConfig() {

    }

    /**
     * Creates a new resource config instance.
     * @param params Resource parameters as array. 1. Name, 2. URI, 3. Resource Type, 4. Address 
     */
    public ResourceConfig(String[] params) {
        m_name = params[0];
        m_uri = params[1];
        m_resourceType = params[2];
        m_address = params[3];
    }

    /**
     * Returns the configured name
     * @return name property
     */
    public String getName() {
        return m_name;
    }

    /**
     * Sets the name
     * @param m_name Resource name
     */
    public void setName(String m_name) {
        this.m_name = m_name;
    }

    /**
     * Returns the configured URI
     * @return Configured URI
     */
    public String getURI() {
        return m_uri;
    }

    /**
     * Sets the configured URI
     * @param m_uri Configuration URI
     */
    public void setURI(String m_uri) {
        this.m_uri = m_uri;
    }

    /**
     * Returns the configured resource type
     * @return configured resource type
     */
    public String getResourceType() {
        return m_resourceType;
    }

    /**
     * Sets the configured resource type
     * @param m_resourceType updates the configured resource type
     */
    public void setResourceType(String m_resourceType) {
        this.m_resourceType = m_resourceType;
    }

    /**
     * Returns the configured address
     * @return Configured address
     */
    public String getAddress() {
        return m_address;
    }

    /**
     * Sets the configured address
     * @param m_address Configured address
     */
    public void setAddress(String m_address) {
        this.m_address = m_address;
    }

    @Override
    public String toString() {
        return "ResourceConfig [m_name=" + m_name + ", m_uri=" + m_uri
                + ", m_resourceType=" + m_resourceType + ", m_address="
                + m_address + "]";
    }

}
