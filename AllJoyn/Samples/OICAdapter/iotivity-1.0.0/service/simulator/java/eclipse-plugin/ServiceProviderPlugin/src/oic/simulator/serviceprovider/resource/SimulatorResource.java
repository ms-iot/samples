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

package oic.simulator.serviceprovider.resource;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import org.oic.simulator.SimulatorResourceModel;
import org.oic.simulator.serviceprovider.AutomationType;
import org.oic.simulator.serviceprovider.ObserverInfo;
import org.oic.simulator.serviceprovider.SimulatorResourceServer;

/**
 * This class represents a simulated resource. It maintains all the necessary
 * information about the resource.
 */
public class SimulatorResource {
    private String                              resourceURI;
    private String                              resourceName;
    private String                              resourceType;
    private String                              resourceInterface;

    // Native Object references
    private SimulatorResourceServer             resourceServer;
    private SimulatorResourceModel              resourceModel;

    private int                                 automationId;

    private boolean                             resourceAutomationInProgress;

    private boolean                             attributeAutomationInProgress;

    private int                                 automationUpdateInterval;

    private AutomationType                      automationType;

    private Map<String, LocalResourceAttribute> resourceAttributesMap;

    private Map<Integer, ObserverDetail>        observers;

    public SimulatorResource() {
        observers = new HashMap<Integer, ObserverDetail>();
    }

    public String getResourceURI() {
        return resourceURI;
    }

    public void setResourceURI(String resourceURI) {
        this.resourceURI = resourceURI;
    }

    public String getResourceName() {
        return resourceName;
    }

    public void setResourceName(String resourceName) {
        this.resourceName = resourceName;
    }

    public String getResourceType() {
        return resourceType;
    }

    public void setResourceType(String resourceType) {
        this.resourceType = resourceType;
    }

    public String getResourceInterface() {
        return resourceInterface;
    }

    public void setResourceInterface(String resourceInterface) {
        this.resourceInterface = resourceInterface;
    }

    public SimulatorResourceServer getResourceServer() {
        return resourceServer;
    }

    public void setResourceServer(SimulatorResourceServer resourceServer) {
        this.resourceServer = resourceServer;
    }

    public SimulatorResourceModel getResourceModel() {
        return resourceModel;
    }

    public void setResourceModel(SimulatorResourceModel resourceModel) {
        this.resourceModel = resourceModel;
    }

    public Map<String, LocalResourceAttribute> getResourceAttributesMap() {
        return resourceAttributesMap;
    }

    public void setResourceAttributesMap(
            Map<String, LocalResourceAttribute> resourceAttributesMap) {
        this.resourceAttributesMap = resourceAttributesMap;
    }

    public int getAutomationUpdateInterval() {
        return automationUpdateInterval;
    }

    public void setAutomationUpdateInterval(int automationUpdateInterval) {
        this.automationUpdateInterval = automationUpdateInterval;
    }

    public AutomationType getAutomationType() {
        return automationType;
    }

    public void setAutomationType(AutomationType automationType) {
        this.automationType = automationType;
    }

    public int getAutomationId() {
        return automationId;
    }

    public void setAutomationId(int automationId) {
        this.automationId = automationId;
    }

    public boolean isResourceAutomationInProgress() {
        return resourceAutomationInProgress;
    }

    public void setResourceAutomationInProgress(
            boolean resourceAutomationInProgress) {
        this.resourceAutomationInProgress = resourceAutomationInProgress;
    }

    public boolean isAttributeAutomationInProgress() {
        return attributeAutomationInProgress;
    }

    public void setAttributeAutomationInProgress(
            boolean attributeAutomationInProgress) {
        this.attributeAutomationInProgress = attributeAutomationInProgress;
    }

    public Map<Integer, ObserverDetail> getObserver() {
        return observers;
    }

    public void addObserverInfo(ObserverInfo observer) {
        if (null == observer) {
            return;
        }
        int id = observer.getId();
        if (!observers.containsKey(id)) {
            ObserverDetail obsDetail = new ObserverDetail();
            obsDetail.setObserverInfo(observer);
            observers.put(id, obsDetail);
        }
    }

    public void removeObserverInfo(ObserverInfo observer) {
        if (null == observer) {
            return;
        }
        observers.remove(observer.getId());
    }

    public void printResourceInfo() {
        System.out.println("Resource URI: " + resourceURI);
        System.out.println("Resource Name: " + resourceName);
        System.out.println("Resource type: " + resourceType);
        System.out.println("Resource Interface: " + resourceInterface);
        System.out.println("Resource Attributes:-");
        if (null != resourceAttributesMap) {
            Iterator<String> attItr = resourceAttributesMap.keySet().iterator();
            while (attItr.hasNext()) {
                resourceAttributesMap.get(attItr.next())
                        .printAttributeDetails();;
            }
        }
    }

    public LocalResourceAttribute getAttribute(String attributeName) {
        if (null == attributeName || null == resourceAttributesMap
                || resourceAttributesMap.size() < 1) {
            return null;
        }
        return resourceAttributesMap.get(attributeName);
    }

}