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

package oic.simulator.serviceprovider.manager;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;

import oic.simulator.serviceprovider.Activator;
import oic.simulator.serviceprovider.listener.IAutomationUIListener;
import oic.simulator.serviceprovider.listener.IObserverListChangedUIListener;
import oic.simulator.serviceprovider.listener.IResourceListChangedUIListener;
import oic.simulator.serviceprovider.listener.IResourceModelChangedUIListener;
import oic.simulator.serviceprovider.listener.IResourceSelectionChangedUIListener;
import oic.simulator.serviceprovider.resource.LocalResourceAttribute;
import oic.simulator.serviceprovider.resource.MetaProperty;
import oic.simulator.serviceprovider.resource.ModelChangeNotificationType;
import oic.simulator.serviceprovider.resource.SimulatorResource;
import oic.simulator.serviceprovider.resource.StandardConfiguration;
import oic.simulator.serviceprovider.utils.Constants;
import oic.simulator.serviceprovider.utils.Utility;

import org.eclipse.swt.graphics.Image;
import org.oic.simulator.IAutomation;
import org.oic.simulator.ILogger.Level;
import org.oic.simulator.InvalidArgsException;
import org.oic.simulator.ResourceAttribute;
import org.oic.simulator.ResourceAttribute.Range;
import org.oic.simulator.ResourceAttribute.Type;
import org.oic.simulator.SimulatorException;
import org.oic.simulator.SimulatorManager;
import org.oic.simulator.SimulatorResourceModel;
import org.oic.simulator.serviceprovider.AutomationType;
import org.oic.simulator.serviceprovider.IObserver;
import org.oic.simulator.serviceprovider.IResourceModelChangedListener;
import org.oic.simulator.serviceprovider.ObserverInfo;
import org.oic.simulator.serviceprovider.SimulatorResourceServer;

/**
 * This class acts as an interface between the simulator java SDK and the
 * various UI modules. It maintains all the details of resources and provides
 * other UI modules with the information required. It also handles model change,
 * automation, and observer related events from native layer and propagates
 * those events to the registered UI listeners.
 */
public class ResourceManager {

    private Map<String, Map<String, SimulatorResource>> resourceMap;

    private Map<String, ArrayList<String>>              orderedResourceUriMap;

    private Map<String, String>                         displayNameMap;

    private StandardConfiguration                       stdConfig;

    private SimulatorResource                           currentResourceInSelection;

    private List<IResourceListChangedUIListener>        resourceListChangedUIListeners;

    private List<IResourceSelectionChangedUIListener>   resourceSelectionChangedUIListeners;

    private List<IResourceModelChangedUIListener>       resourceModelChangedUIListeners;

    private List<IAutomationUIListener>                 automationUIListeners;

    private List<IObserverListChangedUIListener>        observerUIListeners;

    private IResourceModelChangedListener               resourceModelChangeListener;

    private IAutomation                                 automationListener;

    private IObserver                                   observer;

    private NotificationSynchronizerThread              synchronizerThread;

    private Thread                                      threadHandle;

    public ResourceManager() {
        resourceMap = new HashMap<String, Map<String, SimulatorResource>>();
        orderedResourceUriMap = new HashMap<String, ArrayList<String>>();
        displayNameMap = new HashMap<String, String>();
        stdConfig = new StandardConfiguration();

        resourceListChangedUIListeners = new ArrayList<IResourceListChangedUIListener>();
        resourceSelectionChangedUIListeners = new ArrayList<IResourceSelectionChangedUIListener>();
        resourceModelChangedUIListeners = new ArrayList<IResourceModelChangedUIListener>();
        automationUIListeners = new ArrayList<IAutomationUIListener>();
        observerUIListeners = new ArrayList<IObserverListChangedUIListener>();

        resourceModelChangeListener = new IResourceModelChangedListener() {

            @Override
            public void onResourceModelChanged(final String resourceURI,
                    final SimulatorResourceModel resourceModelN) {
                synchronizerThread.addToQueue(new Runnable() {

                    @Override
                    public void run() {
                        if (null == resourceURI || null == resourceModelN) {
                            return;
                        }
                        SimulatorResource resource = getSimulatorResourceByURI(resourceURI);
                        if (null == resource) {
                            return;
                        }

                        ModelChangeNotificationType notificationType;
                        // Fetch the resource attributes
                        Map<String, LocalResourceAttribute> resourceAttributeMapNew;
                        resourceAttributeMapNew = fetchResourceAttributesFromModel(resourceModelN);
                        if (null == resourceAttributeMapNew) {
                            resource.setResourceAttributesMap(null);
                            resourceModelChangedUINotification(
                                    ModelChangeNotificationType.NO_ATTRIBUTES_IN_MODEL,
                                    resourceURI, null);
                            return;
                        }

                        // Update the resource with new model data
                        Map<String, LocalResourceAttribute> resourceAttributeMapOld;
                        resourceAttributeMapOld = resource
                                .getResourceAttributesMap();
                        if (null == resourceAttributeMapOld) {
                            resource.setResourceAttributesMap(resourceAttributeMapNew);
                            resourceModelChangedUINotification(
                                    ModelChangeNotificationType.ATTRIBUTE_ADDED,
                                    resourceURI, null);
                            return;
                        }
                        Set<LocalResourceAttribute> valueChangeSet = new HashSet<LocalResourceAttribute>();
                        notificationType = compareAndUpdateLocalAttributes(
                                resourceAttributeMapOld,
                                resourceAttributeMapNew, valueChangeSet);
                        if (notificationType != ModelChangeNotificationType.NONE) {
                            // Update the UI listeners
                            resourceModelChangedUINotification(
                                    notificationType, resourceURI,
                                    valueChangeSet);
                        }
                    }
                });
            }
        };

        automationListener = new IAutomation() {

            @Override
            public void onAutomationComplete(final String resourceURI,
                    final int automationId) {
                synchronizerThread.addToQueue(new Runnable() {

                    @Override
                    public void run() {
                        SimulatorResource resource = getSimulatorResourceByURI(resourceURI);
                        if (null == resource) {
                            return;
                        }
                        // Checking whether this notification is for an
                        // attribute or a resource
                        if (resource.isResourceAutomationInProgress()) {
                            changeResourceLevelAutomationStatus(resource, false);
                            // Notify the UI listeners
                            automationCompleteUINotification(resourceURI, null);
                        } else if (resource.isAttributeAutomationInProgress()) {
                            // Find the attribute with the given automation id
                            LocalResourceAttribute attribute;
                            attribute = getAttributeWithGivenAutomationId(
                                    resource, automationId);
                            if (null != attribute) {
                                attribute.setAutomationInProgress(false);
                                resource.setAttributeAutomationInProgress(isAnyAttributeInAutomation(resource));
                                // Notify the UI listeners
                                automationCompleteUINotification(resourceURI,
                                        attribute.getAttributeName());
                            }
                        } else {
                            // Ignoring the notification as there are no
                            // known automation for the current resource.
                        }
                    }
                });
            }
        };

        observer = new IObserver() {

            @Override
            public void onObserverChanged(final String resourceURI,
                    final int status, final ObserverInfo observer) {
                new Thread() {
                    @Override
                    public void run() {
                        if (null == resourceURI || null == observer) {
                            return;
                        }
                        SimulatorResource resource = getSimulatorResourceByURI(resourceURI);
                        if (null == resource) {
                            return;
                        }
                        // Update the observers information
                        if (status == 0) {
                            resource.addObserverInfo(observer);
                        } else {
                            resource.removeObserverInfo(observer);
                        }
                        // Notify the UI listeners
                        observerListChangedUINotification(resourceURI);
                    }
                }.start();
            }
        };

        synchronizerThread = new NotificationSynchronizerThread();
        threadHandle = new Thread(synchronizerThread);
        threadHandle.setName("Simulator service provider event queue");
        threadHandle.start();
    }

    private static class NotificationSynchronizerThread implements Runnable {

        LinkedList<Runnable> notificationQueue = new LinkedList<Runnable>();

        @Override
        public void run() {
            while (!Thread.interrupted()) {
                synchronized (this) {
                    try {
                        while (notificationQueue.isEmpty()) {
                            this.wait();
                            break;
                        }
                    } catch (InterruptedException e) {
                        return;
                    }
                }

                Runnable thread;
                synchronized (this) {
                    thread = notificationQueue.pop();
                }
                try {
                    thread.run();
                } catch (Exception e) {
                    if (e instanceof InterruptedException) {
                        return;
                    }
                    e.printStackTrace();
                }
            }
        }

        public void addToQueue(Runnable event) {
            synchronized (this) {
                notificationQueue.add(event);
                this.notify();
            }
        }
    }

    // This method gives a list of available RAML resource configurations.
    public Map<String, String> getResourceConfigurationList() {
        return stdConfig.getStandardResourceConfigurationList();
    }

    public String getConfigFilePath(String fileName) {
        return stdConfig.getFilePath(fileName);
    }

    public void addResourceListChangedUIListener(
            IResourceListChangedUIListener resourceListChangedUIListener) {
        synchronized (resourceListChangedUIListeners) {
            resourceListChangedUIListeners.add(resourceListChangedUIListener);
        }
    }

    public void addResourceSelectionChangedUIListener(
            IResourceSelectionChangedUIListener resourceSelectionChangedUIListener) {
        synchronized (resourceSelectionChangedUIListeners) {
            resourceSelectionChangedUIListeners
                    .add(resourceSelectionChangedUIListener);
        }
    }

    public void addResourceModelChangedUIListener(
            IResourceModelChangedUIListener resourceModelChangedUIListener) {
        synchronized (resourceModelChangedUIListeners) {
            resourceModelChangedUIListeners.add(resourceModelChangedUIListener);
        }
    }

    public void addAutomationUIListener(
            IAutomationUIListener automationUIListener) {
        synchronized (automationUIListeners) {
            automationUIListeners.add(automationUIListener);
        }
    }

    public void addObserverListChangedUIListener(
            IObserverListChangedUIListener observerListChangedUIListener) {
        synchronized (observerUIListeners) {
            observerUIListeners.add(observerListChangedUIListener);
        }
    }

    public void removeResourceListChangedUIListener(
            IResourceListChangedUIListener listener) {
        synchronized (resourceListChangedUIListeners) {
            if (null != listener && resourceListChangedUIListeners.size() > 0) {
                resourceListChangedUIListeners.remove(listener);
            }
        }
    }

    public void removeResourceSelectionChangedUIListener(
            IResourceSelectionChangedUIListener listener) {
        synchronized (resourceSelectionChangedUIListeners) {
            if (null != listener
                    && resourceSelectionChangedUIListeners.size() > 0) {
                resourceSelectionChangedUIListeners.remove(listener);
            }
        }
    }

    public void removeResourceModelChangedUIListener(
            IResourceModelChangedUIListener listener) {
        synchronized (resourceModelChangedUIListeners) {
            if (null != listener && resourceModelChangedUIListeners.size() > 0) {
                resourceModelChangedUIListeners.remove(listener);
            }
        }
    }

    public void removeAutomationUIListener(IAutomationUIListener listener) {
        synchronized (automationUIListeners) {
            if (null != listener && automationUIListeners.size() > 0) {
                automationUIListeners.remove(listener);
            }
        }
    }

    public void removeObserverListChangedUIListener(
            IObserverListChangedUIListener listener) {
        synchronized (observerUIListeners) {
            if (null != listener && observerUIListeners.size() > 0) {
                observerUIListeners.remove(listener);
            }
        }
    }

    public synchronized SimulatorResource getCurrentResourceInSelection() {
        return currentResourceInSelection;
    }

    public synchronized void setCurrentResourceInSelection(
            SimulatorResource resource) {
        this.currentResourceInSelection = resource;
    }

    private void addResourceUriToOrderedMap(String resourceType,
            String resourceURI) {
        if (null != resourceURI && null != resourceType) {
            synchronized (orderedResourceUriMap) {
                ArrayList<String> uriListForType = orderedResourceUriMap
                        .get(resourceType);
                if (null == uriListForType) {
                    uriListForType = new ArrayList<String>();
                    orderedResourceUriMap.put(resourceType, uriListForType);
                }
                uriListForType.add(resourceURI);
            }
        }
    }

    private void removeResourceUriFromOrderedMap(String resourceType,
            String resourceURI) {
        synchronized (orderedResourceUriMap) {
            if (null != resourceURI && null != resourceType) {
                ArrayList<String> uriListForType = orderedResourceUriMap
                        .get(resourceType);
                if (null != uriListForType) {
                    uriListForType.remove(resourceURI);
                    if (uriListForType.size() < 1) {
                        orderedResourceUriMap.remove(resourceType);
                    }
                }
            } else if (null != resourceType) {
                orderedResourceUriMap.remove(resourceType);
            } else {
                orderedResourceUriMap.clear();
            }
        }
    }

    private void addResourceToMap(SimulatorResource simulatorResource) {
        if (null != simulatorResource) {
            synchronized (resourceMap) {
                Map<String, SimulatorResource> resourceTypeMap;
                resourceTypeMap = resourceMap.get(simulatorResource
                        .getResourceType());
                if (null == resourceTypeMap) {
                    resourceTypeMap = new HashMap<String, SimulatorResource>();
                    resourceMap.put(simulatorResource.getResourceType(),
                            resourceTypeMap);
                }
                resourceTypeMap.put(simulatorResource.getResourceURI(),
                        simulatorResource);
            }
        }
    }

    private void addResourceToMap(String resourceType,
            Map<String, SimulatorResource> newResourceTypeMap) {
        if (null != resourceType && null != newResourceTypeMap) {
            synchronized (resourceMap) {
                Map<String, SimulatorResource> resourceTypeMap = resourceMap
                        .get(resourceType);
                if (null != resourceTypeMap) {
                    resourceTypeMap.putAll(newResourceTypeMap);
                } else {
                    resourceMap.put(resourceType, newResourceTypeMap);
                }
            }
        }
    }

    private void removeResourceFromMap(String resourceType, String resourceURI) {
        if (null != resourceURI && null != resourceType) {
            synchronized (resourceMap) {
                Map<String, SimulatorResource> resourceTypeMap = resourceMap
                        .get(resourceType);
                if (null != resourceTypeMap) {
                    resourceTypeMap.remove(resourceURI);
                    if (resourceTypeMap.size() < 1) {
                        resourceMap.remove(resourceType);
                    }
                }
            }
        }
    }

    public boolean isResourceExist(String resourceURI) {
        boolean result = false;
        if (null != resourceURI) {
            SimulatorResource resource = getSimulatorResourceByURI(resourceURI);
            if (null != resource) {
                result = true;
            }
        }
        return result;
    }

    public void addResourceDisplayName(String displayName, String completeURI) {
        synchronized (displayNameMap) {
            displayNameMap.put(displayName, completeURI);
        }
    }

    public void removeResourceDisplayName(String displayName) {
        synchronized (displayNameMap) {
            displayNameMap.remove(displayName);
        }
    }

    public boolean isDisplayName(String displayName) {
        boolean exist;
        synchronized (displayNameMap) {
            exist = displayNameMap.containsKey(displayName);
        }
        return exist;
    }

    public String getCompleteUriFromDisplayName(String displayName) {
        String completeURI;
        synchronized (displayNameMap) {
            completeURI = displayNameMap.get(displayName);
        }
        return completeURI;
    }

    public void createResource(final String configFilePath) {
        new Thread() {
            @Override
            public void run() {
                SimulatorResourceServer resourceServerN;
                try {
                    resourceServerN = SimulatorManager.createResource(
                            configFilePath, resourceModelChangeListener);
                } catch (SimulatorException e) {
                    Activator
                            .getDefault()
                            .getLogManager()
                            .log(Level.ERROR.ordinal(),
                                    new Date(),
                                    "[" + e.getClass().getSimpleName() + "]"
                                            + e.code().toString() + "-"
                                            + e.message());
                    return;
                }
                SimulatorResource simulatorResource;
                simulatorResource = fetchResourceData(resourceServerN);
                if (null != simulatorResource) {
                    String uri = simulatorResource.getResourceURI();
                    addResourceToMap(simulatorResource);
                    addResourceUriToOrderedMap(
                            simulatorResource.getResourceType(), uri);
                    resourceCreatedUINotification();

                    // Form the display URI
                    String displayName = Utility.uriToDisplayName(uri);
                    if (null != displayName) {
                        addResourceDisplayName(displayName, uri);
                    } else {
                        Activator
                                .getDefault()
                                .getLogManager()
                                .log(Level.ERROR.ordinal(),
                                        new Date(),
                                        "Converting the URI to display format for "
                                                + uri + "failed!");
                    }

                    // Set the observer for the created resource
                    try {
                        resourceServerN.setObserverCallback(observer);
                    } catch (SimulatorException e) {
                        Activator
                                .getDefault()
                                .getLogManager()
                                .log(Level.ERROR.ordinal(),
                                        new Date(),
                                        "[" + e.getClass().getSimpleName()
                                                + "]" + e.code().toString()
                                                + "-" + e.message());
                    }

                    // Print the resource data
                    simulatorResource.printResourceInfo();
                }
            }
        }.start();
    }

    public void createResource(final String configFilePath,
            final int noOfInstances) {
        new Thread() {
            @Override
            public void run() {
                Map<String, SimulatorResource> resourceTypeMap;
                SimulatorResourceServer[] simulatorResourceServers;
                try {
                    simulatorResourceServers = SimulatorManager.createResource(
                            configFilePath, noOfInstances,
                            resourceModelChangeListener);
                } catch (SimulatorException e) {
                    Activator
                            .getDefault()
                            .getLogManager()
                            .log(Level.ERROR.ordinal(),
                                    new Date(),
                                    "[" + e.getClass().getSimpleName() + "]"
                                            + e.code().toString() + "-"
                                            + e.message());
                    return;
                }
                if (null == simulatorResourceServers) {
                    return;
                }
                resourceTypeMap = new HashMap<String, SimulatorResource>();
                SimulatorResource resource;
                String uri;
                String displayName;
                for (SimulatorResourceServer resourceServerN : simulatorResourceServers) {
                    resource = fetchResourceData(resourceServerN);
                    if (null != resource) {
                        uri = resource.getResourceURI();
                        resourceTypeMap.put(uri, resource);
                        addResourceUriToOrderedMap(resource.getResourceType(),
                                uri);

                        // Form the display URI
                        displayName = Utility.uriToDisplayName(uri);
                        if (null != displayName) {
                            addResourceDisplayName(displayName, uri);
                        } else {
                            Activator
                                    .getDefault()
                                    .getLogManager()
                                    .log(Level.ERROR.ordinal(),
                                            new Date(),
                                            "Converting the URI to display format for "
                                                    + uri + "failed!");
                        }
                    }
                    // Set the observer for the created resource
                    try {
                        resourceServerN.setObserverCallback(observer);
                    } catch (SimulatorException e) {
                        Activator
                                .getDefault()
                                .getLogManager()
                                .log(Level.ERROR.ordinal(),
                                        new Date(),
                                        "[" + e.getClass().getSimpleName()
                                                + "]" + e.code().toString()
                                                + "-" + e.message());
                    }
                }

                // Find the resourceType and add it to the local data
                // structure and notify UI Listeners
                if (resourceTypeMap.size() > 0) {
                    String resourceType;
                    Set<String> uriSet = resourceTypeMap.keySet();
                    Iterator<String> itr = uriSet.iterator();
                    if (itr.hasNext()) {
                        SimulatorResource simResource = resourceTypeMap.get(itr
                                .next());
                        if (null != simResource) {
                            resourceType = simResource.getResourceType();

                            addResourceToMap(resourceType, resourceTypeMap);
                            resourceCreatedUINotification();
                        }
                    }
                }
            }
        }.start();
    }

    private SimulatorResource fetchResourceData(
            SimulatorResourceServer resourceServerN) {
        SimulatorResource simulatorResource = null;
        if (null != resourceServerN) {
            simulatorResource = new SimulatorResource();
            simulatorResource.setResourceServer(resourceServerN);
            simulatorResource.setResourceURI(resourceServerN.getURI());
            simulatorResource
                    .setResourceType(resourceServerN.getResourceType());
            simulatorResource.setResourceName(resourceServerN.getName());
            simulatorResource.setResourceInterface(resourceServerN
                    .getInterfaceType());

            SimulatorResourceModel resourceModelN;
            try {
                resourceModelN = resourceServerN.getModel();
            } catch (SimulatorException e) {
                Activator
                        .getDefault()
                        .getLogManager()
                        .log(Level.ERROR.ordinal(),
                                new Date(),
                                "[" + e.getClass().getSimpleName() + "]"
                                        + e.code().toString() + "-"
                                        + e.message());
                return null;
            }
            if (null != resourceModelN) {
                simulatorResource.setResourceModel(resourceModelN);

                // Fetch the resource attributes
                Map<String, LocalResourceAttribute> resourceAttributeMap;
                resourceAttributeMap = fetchResourceAttributesFromModel(resourceModelN);
                if (null != resourceAttributeMap) {
                    simulatorResource
                            .setResourceAttributesMap(resourceAttributeMap);
                }
            }
        }
        return simulatorResource;
    }

    private Map<String, LocalResourceAttribute> fetchResourceAttributesFromModel(
            SimulatorResourceModel resourceModelN) {
        Map<String, LocalResourceAttribute> resourceAttributeMap = null;
        if (null != resourceModelN) {
            Map<String, ResourceAttribute> attributeMapN;
            try {
                attributeMapN = resourceModelN.getAttributes();
            } catch (SimulatorException e) {
                Activator
                        .getDefault()
                        .getLogManager()
                        .log(Level.ERROR.ordinal(),
                                new Date(),
                                "[" + e.getClass().getSimpleName() + "]"
                                        + e.code().toString() + "-"
                                        + e.message());
                return null;
            }
            if (null != attributeMapN) {
                resourceAttributeMap = new HashMap<String, LocalResourceAttribute>();

                Set<String> attNameSet = attributeMapN.keySet();
                String attName;
                ResourceAttribute attributeN;
                LocalResourceAttribute attribute;
                Iterator<String> attNameItr = attNameSet.iterator();
                while (attNameItr.hasNext()) {
                    attName = attNameItr.next();
                    attributeN = attributeMapN.get(attName);
                    if (null != attributeN) {
                        attribute = new LocalResourceAttribute();
                        attribute.setResourceAttribute(attributeN);

                        // Set the attribute value
                        Object valueObj = attributeN.getValue();
                        if (null != valueObj) {
                            attribute.setAttributeValue(valueObj);
                        }

                        // Set the attribute value list.
                        attribute.setAttValues(getValueList(attributeN));

                        // Initially disabling the automation
                        attribute.setAutomationInProgress(false);

                        // TODO: Temporarily setting the interval to 500.
                        // This value should come from the native layer.
                        // Native implementation is in progress.
                        attribute
                                .setAutomationUpdateInterval(Constants.DEFAULT_AUTOMATION_INTERVAL);

                        // Setting the default automation type
                        attribute
                                .setAutomationType(Constants.DEFAULT_AUTOMATION_TYPE);

                        resourceAttributeMap.put(attName, attribute);
                    }
                }
            }
        }
        return resourceAttributeMap;
    }

    // This method gives all known possible values of the attribute in string
    // format. It takes allowed values or range of values whichever is available
    private List<String> getValueList(ResourceAttribute attributeN) {
        Object[] allowedValues = attributeN.getAllowedValues();
        List<String> valueList = new ArrayList<String>();
        if (null != allowedValues && allowedValues.length > 0) {
            for (Object value : allowedValues) {
                if (null != value) {
                    valueList.add(String.valueOf(value));
                }
            }
        } else {
            Type valueType = attributeN.getBaseType();
            Range range = attributeN.getRange();
            if (null != range) {
                Object min = range.getMin();
                Object max = range.getMax();
                if (valueType == Type.INT) {
                    int minI = (Integer) min;
                    int maxI = (Integer) max;
                    for (int value = minI; value <= maxI; value++) {
                        valueList.add(String.valueOf(value));
                    }
                } else if (valueType == Type.DOUBLE) {
                    double minD = (Double) min;
                    double maxD = (Double) max;
                    for (double value = minD; value <= maxD; value++) {
                        valueList.add(String.valueOf(value));
                    }
                }
            }
        }
        Object attValue = attributeN.getValue();
        if (valueList.size() < 1 && null != attValue) {
            valueList.add(String.valueOf(attValue));
        }
        return valueList;
    }

    public void deleteResourceByURI(final String resourceURI) {
        if (null != resourceURI) {
            new Thread() {
                @Override
                public void run() {
                    SimulatorResource resource = getSimulatorResourceByURI(resourceURI);
                    if (null != resource) {
                        String resourceType = resource.getResourceType();

                        // Unregister the resource from the platform
                        deleteResource(resource);

                        // Delete from the local data structure
                        deleteLocalResourceDetails(resourceType, resourceURI);

                        // Notify the UI listener for removing this resource
                        // from UI
                        resourceDeletedUINotification();

                        if (null != currentResourceInSelection
                                && resource == currentResourceInSelection) {
                            // Listeners might query the resource being deleted
                            // if exists. So set the currently selection to
                            // null.
                            setCurrentResourceInSelection(null);

                            // Notify all observers for resource selection
                            // change event
                            resourceSelectionChangedUINotification();
                        }
                    }
                }
            }.start();
        }
    }

    private SimulatorResource getSimulatorResourceByURI(String resourceURI) {
        SimulatorResource resource = null;
        if (null != resourceURI) {
            synchronized (resourceMap) {
                Set<String> typeSet = resourceMap.keySet();
                Iterator<String> typeItr = typeSet.iterator();
                String resourceType;
                Map<String, SimulatorResource> resourceTypeMap;
                while (typeItr.hasNext()) {
                    resourceType = typeItr.next();
                    resourceTypeMap = resourceMap.get(resourceType);
                    if (null != resourceTypeMap) {
                        resource = resourceTypeMap.get(resourceURI);
                        if (null != resource) {
                            break;
                        }
                    }
                }
            }
        }
        return resource;
    }

    private void deleteResource(SimulatorResource resource) {
        if (null != resource) {
            SimulatorResourceServer resourceServerN = resource
                    .getResourceServer();
            if (null != resourceServerN) {
                try {
                    SimulatorManager.deleteResource(resourceServerN);
                } catch (SimulatorException e) {
                    Activator
                            .getDefault()
                            .getLogManager()
                            .log(Level.ERROR.ordinal(),
                                    new Date(),
                                    "[" + e.getClass().getSimpleName() + "]"
                                            + e.code().toString() + "-"
                                            + e.message());
                }
            }
        }
    }

    public void deleteResourceByType(final String resourceType) {
        if (null != resourceType) {
            new Thread() {
                @Override
                public void run() {
                    // Unregister the resources from the platform
                    deleteResource(resourceType);

                    // Delete from the local data structure
                    deleteLocalResourceDetails(resourceType, null);

                    // Notify the UI listener for removing this resource from UI
                    resourceDeletedUINotification();

                    if (null != currentResourceInSelection
                            && resourceType.equals(currentResourceInSelection
                                    .getResourceType())) {
                        // Listeners might query the resource being deleted if
                        // exists. So set the currently selection to null.
                        setCurrentResourceInSelection(null);

                        // Notify all observers for resource selection change
                        // event
                        resourceSelectionChangedUINotification();
                    }
                }
            }.start();
        }
    }

    private void deleteResource(String resourceType) {
        if (null != resourceType) {
            try {
                SimulatorManager.deleteResources(resourceType);
            } catch (SimulatorException e) {
                Activator
                        .getDefault()
                        .getLogManager()
                        .log(Level.ERROR.ordinal(),
                                new Date(),
                                "[" + e.getClass().getSimpleName() + "]"
                                        + e.code().toString() + "-"
                                        + e.message());
            }
        }
    }

    public void deleteAllResources() {
        new Thread() {
            @Override
            public void run() {
                // Unregister the resources from the platform
                deleteResource();

                // Delete from the local data structure
                deleteLocalResourceDetails(null, null);

                // Notify the UI listener for removing this resource from UI
                resourceDeletedUINotification();

                // Listeners might query the resource being deleted if exists.
                // So set the currently selection to null.
                setCurrentResourceInSelection(null);

                // Notify all observers for resource selection change event
                resourceSelectionChangedUINotification();
            }
        }.start();
    }

    private void deleteResource() {
        try {
            SimulatorManager.deleteResources(null);
        } catch (SimulatorException e) {
            Activator
                    .getDefault()
                    .getLogManager()
                    .log(Level.ERROR.ordinal(),
                            new Date(),
                            "[" + e.getClass().getSimpleName() + "]"
                                    + e.code().toString() + "-" + e.message());
        }
    }

    private void deleteLocalResourceDetails(String resourceType,
            String resourceURI) {
        if (null != resourceType && null != resourceURI) {
            removeResourceFromMap(resourceType, resourceURI);
            removeResourceUriFromOrderedMap(resourceType, resourceURI);
        } else {
            synchronized (resourceMap) {
                if (null != resourceType) {
                    removeResourceUriFromOrderedMap(resourceType, null);
                    resourceMap.remove(resourceType);
                } else {
                    resourceMap.clear();
                    removeResourceUriFromOrderedMap(null, null);
                }
            }
        }
        // Remove the display name from the map
        if (null != resourceURI) {
            String dispName = Utility.uriToDisplayName(resourceURI);
            if (null != dispName)
                removeResourceDisplayName(dispName);
        }
    }

    private void resourceCreatedUINotification() {
        synchronized (resourceListChangedUIListeners) {
            if (resourceListChangedUIListeners.size() > 0) {
                IResourceListChangedUIListener listener;
                Iterator<IResourceListChangedUIListener> listenerItr = resourceListChangedUIListeners
                        .iterator();
                while (listenerItr.hasNext()) {
                    listener = listenerItr.next();
                    if (null != listener) {
                        listener.onResourceCreation();
                    }
                }
            }
        }
    }

    private void resourceDeletedUINotification() {
        synchronized (resourceListChangedUIListeners) {
            if (resourceListChangedUIListeners.size() > 0) {
                IResourceListChangedUIListener listener;
                Iterator<IResourceListChangedUIListener> listenerItr = resourceListChangedUIListeners
                        .iterator();
                while (listenerItr.hasNext()) {
                    listener = listenerItr.next();
                    if (null != listener) {
                        listener.onResourceDeletion();
                    }
                }
            }
        }
    }

    private void resourceSelectionChangedUINotification() {
        synchronized (resourceSelectionChangedUIListeners) {
            if (resourceSelectionChangedUIListeners.size() > 0) {
                IResourceSelectionChangedUIListener listener;
                Iterator<IResourceSelectionChangedUIListener> listenerItr = resourceSelectionChangedUIListeners
                        .iterator();
                while (listenerItr.hasNext()) {
                    listener = listenerItr.next();
                    if (null != listener) {
                        listener.onResourceSelectionChange();
                    }
                }
            }
        }
    }

    private void resourceModelChangedUINotification(
            ModelChangeNotificationType notificationType, String resourceURI,
            Set<LocalResourceAttribute> valueChangeSet) {
        synchronized (resourceModelChangedUIListeners) {
            if (resourceModelChangedUIListeners.size() > 0
                    && notificationType != ModelChangeNotificationType.NONE
                    && null != resourceURI) {
                IResourceModelChangedUIListener listener;
                Iterator<IResourceModelChangedUIListener> listenerItr = resourceModelChangedUIListeners
                        .iterator();
                while (listenerItr.hasNext()) {
                    listener = listenerItr.next();
                    if (null != listener) {
                        listener.onResourceModelChange(notificationType,
                                resourceURI, valueChangeSet);
                    }
                }
            }
        }
    }

    private void resourceAutomationStartedUINotification(String resourceURI) {
        synchronized (automationUIListeners) {
            if (automationUIListeners.size() > 0 && null != resourceURI) {
                IAutomationUIListener listener;
                Iterator<IAutomationUIListener> listenerItr = automationUIListeners
                        .iterator();
                while (listenerItr.hasNext()) {
                    listener = listenerItr.next();
                    if (null != listener) {
                        listener.onResourceAutomationStart(resourceURI);
                    }
                }
            }
        }
    }

    private void automationCompleteUINotification(String resourceURI,
            String attName) {
        synchronized (automationUIListeners) {
            if (automationUIListeners.size() > 0 && null != resourceURI) {
                IAutomationUIListener listener;
                Iterator<IAutomationUIListener> listenerItr = automationUIListeners
                        .iterator();
                while (listenerItr.hasNext()) {
                    listener = listenerItr.next();
                    if (null != listener) {
                        listener.onAutomationComplete(resourceURI, attName);
                    }
                }
            }
        }
    }

    private void observerListChangedUINotification(String resourceURI) {
        synchronized (observerUIListeners) {
            if (observerUIListeners.size() > 0 && null != resourceURI) {
                IObserverListChangedUIListener listener;
                Iterator<IObserverListChangedUIListener> listenerItr = observerUIListeners
                        .iterator();
                while (listenerItr.hasNext()) {
                    listener = listenerItr.next();
                    if (null != listener) {
                        listener.onObserverListChanged(resourceURI);
                    }
                }
            }
        }
    }

    public List<String> getResourceTypeList() {
        List<String> typeList = null;
        synchronized (resourceMap) {
            if (resourceMap.size() > 0) {
                typeList = new ArrayList<String>();
                Set<String> typeSet = resourceMap.keySet();
                Iterator<String> typeItr = typeSet.iterator();
                while (typeItr.hasNext()) {
                    typeList.add(typeItr.next());
                }
            }
        }
        return typeList;
    }

    public boolean isTypeExist(String resType) {
        synchronized (resourceMap) {
            if (resourceMap.containsKey(resType)) {
                return true;
            }
        }
        return false;
    }

    public List<String> getURIList() {
        List<String> list;
        synchronized (orderedResourceUriMap) {
            Set<String> typeSet = orderedResourceUriMap.keySet();
            List<String> typeList = Utility.convertSetToList(typeSet);
            if (null == typeList || typeList.size() < 1) {
                return null;
            }
            list = new ArrayList<String>();

            // Sort the types
            Collections.sort(typeList);

            // Add all URIs to the output list
            Iterator<String> typeItr = typeList.iterator();
            while (typeItr.hasNext()) {
                List<String> l = orderedResourceUriMap.get(typeItr.next());
                if (null != l) {
                    list.addAll(l);
                }
            }
        }
        return list;
    }

    public void resourceSelectionChanged(final String selectedItem) {
        new Thread() {
            @Override
            public void run() {
                // Check whether the item selected is a resource or resource
                // category
                if (isTypeExist(selectedItem)) {
                    // Given item is a resource Type
                    setCurrentResourceInSelection(null);
                } else {
                    // Given item is a resource URI
                    SimulatorResource resource = getSimulatorResourceByURI(selectedItem);
                    if (null != resource) {
                        setCurrentResourceInSelection(resource);
                    } else {
                        setCurrentResourceInSelection(null);
                    }
                }
                // Notify all observers for resource selection change event
                resourceSelectionChangedUINotification();
            }
        }.start();
    }

    public List<MetaProperty> getMetaProperties(SimulatorResource resource) {
        if (null != resource) {
            String propName;
            String propValue;

            List<MetaProperty> metaPropertyList = new ArrayList<MetaProperty>();

            for (int index = 0; index < Constants.META_PROPERTY_COUNT; index++) {
                propName = Constants.META_PROPERTIES[index];
                if (propName.equals(Constants.RESOURCE_URI)) {
                    propValue = resource.getResourceURI();
                } else if (propName.equals(Constants.RESOURCE_TYPE)) {
                    propValue = resource.getResourceType();
                } else if (propName.equals(Constants.CONNECTIVITY_TYPE)) {
                    // TODO: Temporarily ignoring till the implementation.
                    propValue = null;
                } else {
                    propValue = null;
                }
                if (null != propValue) {
                    metaPropertyList.add(new MetaProperty(propName, propValue));
                }
            }

            return metaPropertyList;
        }
        return null;
    }

    public List<LocalResourceAttribute> getAttributes(SimulatorResource resource) {
        List<LocalResourceAttribute> attList = null;
        if (null != resource) {
            Map<String, LocalResourceAttribute> attMap = resource
                    .getResourceAttributesMap();
            if (null != attMap && attMap.size() > 0) {
                attList = new ArrayList<LocalResourceAttribute>();
                Set<String> attNameSet = attMap.keySet();
                String attName;
                LocalResourceAttribute attribute;
                Iterator<String> attNameItr = attNameSet.iterator();
                while (attNameItr.hasNext()) {
                    attName = attNameItr.next();
                    attribute = attMap.get(attName);
                    if (null != attribute) {
                        attList.add(attribute);
                    }
                }
            }
        }
        return attList;
    }

    public void attributeValueUpdated(SimulatorResource resource,
            String attributeName, String value) {
        if (null != resource && null != attributeName && null != value) {
            SimulatorResourceServer server = resource.getResourceServer();
            if (null != server) {
                LocalResourceAttribute att = resource
                        .getAttribute(attributeName);
                if (null == att) {
                    return;
                }
                Type baseType = att.getAttValBaseType();
                try {
                    if (baseType == Type.STRING) {
                        server.updateAttributeString(attributeName, value);
                    } else if (baseType == Type.INT) {
                        int val;
                        try {
                            val = Integer.parseInt(value);
                            server.updateAttributeInteger(attributeName, val);
                        } catch (NumberFormatException nfe) {
                            return;
                        }
                    } else if (baseType == Type.DOUBLE) {
                        double val;
                        try {
                            val = Double.parseDouble(value);
                            server.updateAttributeDouble(attributeName, val);
                        } catch (NumberFormatException nfe) {
                            return;
                        }
                    } else if (baseType == Type.BOOL) {
                        boolean val;
                        val = Boolean.parseBoolean(value);
                        server.updateAttributeBoolean(attributeName, val);
                    }
                } catch (SimulatorException e) {
                    Activator
                            .getDefault()
                            .getLogManager()
                            .log(Level.ERROR.ordinal(),
                                    new Date(),
                                    "[" + e.getClass().getSimpleName() + "]"
                                            + e.code().toString() + "-"
                                            + e.message());
                }
            }
        }
    }

    private ModelChangeNotificationType compareAndUpdateLocalAttributes(
            Map<String, LocalResourceAttribute> resourceAttributeMapOld,
            Map<String, LocalResourceAttribute> resourceAttributeMapNew,
            Set<LocalResourceAttribute> valueChangeSet) {
        ModelChangeNotificationType notificationType = ModelChangeNotificationType.NONE;
        if (null != resourceAttributeMapOld && null != resourceAttributeMapNew) {
            Set<String> oldMapKeySet = resourceAttributeMapOld.keySet();
            Iterator<String> attributeMapOldItr = oldMapKeySet.iterator();
            String attName;
            LocalResourceAttribute attributeOld;
            LocalResourceAttribute attributeNew;
            Object attValueOld;
            Object attValueNew;
            String oldValueStr;
            String newValueStr;
            while (attributeMapOldItr.hasNext()) {
                attName = attributeMapOldItr.next();
                if (resourceAttributeMapNew.containsKey(attName)) {
                    attributeOld = resourceAttributeMapOld.get(attName);
                    attributeNew = resourceAttributeMapNew.get(attName);
                    // Copy the attribute value from new to old if the value
                    // has been changed
                    // Comparing only the attribute's value considering the
                    // fact that only the value can be changed
                    if (null != attributeOld && null != attributeNew) {
                        attValueOld = attributeOld.getAttributeValue();
                        attValueNew = attributeNew.getAttributeValue();

                        oldValueStr = String.valueOf(attValueOld);
                        newValueStr = String.valueOf(attValueNew);

                        if (null != oldValueStr && null != newValueStr) {
                            if (!oldValueStr.equals(newValueStr)) {
                                attributeOld.setAttributeValue(attValueNew);
                                notificationType = ModelChangeNotificationType.ATTRIBUTE_VALUE_CHANGED;
                                valueChangeSet.add(attributeOld);
                            }
                        }
                    }
                    resourceAttributeMapNew.remove(attName);
                } else {
                    // Attribute doesn't exist in the new model. Hence
                    // removing it from the model.
                    resourceAttributeMapOld.remove(attName);
                    notificationType = ModelChangeNotificationType.ATTRIBUTE_REMOVED;
                }
            }
            // Check for new attributes in the new model
            if (resourceAttributeMapNew.size() > 0) {
                Set<String> remainingAttSet = resourceAttributeMapNew.keySet();
                Iterator<String> remainingAttItr = remainingAttSet.iterator();
                LocalResourceAttribute attribute;
                while (remainingAttItr.hasNext()) {
                    attName = remainingAttItr.next();
                    if (null != attName) {
                        attribute = resourceAttributeMapNew.get(attName);
                        if (null != attribute) {
                            resourceAttributeMapOld.put(attName, attribute);
                        }
                    }
                }
                notificationType = ModelChangeNotificationType.ATTRIBUTE_ADDED;
            }
        }
        return notificationType;
    }

    public int startAutomation(SimulatorResource resource,
            LocalResourceAttribute attribute, AutomationType autoType,
            int autoUpdateInterval) {
        int autoId = -1;
        if (null != resource && null != attribute) {
            SimulatorResourceServer resourceServerN = resource
                    .getResourceServer();
            if (null != resourceServerN) {
                String attrName = attribute.getAttributeName();
                try {
                    autoId = resourceServerN.startAttributeAutomation(attrName,
                            autoType, automationListener);
                } catch (SimulatorException e) {
                    Activator
                            .getDefault()
                            .getLogManager()
                            .log(Level.ERROR.ordinal(),
                                    new Date(),
                                    "[" + e.getClass().getSimpleName() + "]"
                                            + e.code().toString() + "-"
                                            + e.message());
                    return -1;
                }
                if (-1 != autoId) {
                    attribute.setAutomationId(autoId);
                    attribute.setAutomationType(autoType);
                    attribute.setAutomationUpdateInterval(autoUpdateInterval);
                    attribute.setAutomationInProgress(true);
                    resource.setAttributeAutomationInProgress(true);
                } else {
                    attribute.setAutomationInProgress(false);
                }
            }
        }
        return autoId;
    }

    public void stopAutomation(SimulatorResource resource,
            LocalResourceAttribute att, int autoId) {
        if (null != resource) {
            SimulatorResourceServer resourceServerN = resource
                    .getResourceServer();
            if (null != resourceServerN) {
                try {
                    resourceServerN.stopAutomation(autoId);
                } catch (SimulatorException e) {
                    Activator
                            .getDefault()
                            .getLogManager()
                            .log(Level.ERROR.ordinal(),
                                    new Date(),
                                    "[" + e.getClass().getSimpleName() + "]"
                                            + e.code().toString() + "-"
                                            + e.message());
                    return;
                }
                // Change the automation status
                att.setAutomationInProgress(false);
                resource.setAttributeAutomationInProgress(isAnyAttributeInAutomation(resource));
            }
        }
    }

    private boolean isAnyAttributeInAutomation(SimulatorResource resource) {
        if (null == resource) {
            return false;
        }
        Map<String, LocalResourceAttribute> attMap = resource
                .getResourceAttributesMap();
        if (null == attMap) {
            return false;
        }
        boolean status = false;
        Set<String> keySet = attMap.keySet();
        Iterator<String> attItr = keySet.iterator();
        while (attItr.hasNext()) {
            LocalResourceAttribute attribute = attMap.get(attItr.next());
            if (attribute.isAutomationInProgress()) {
                status = true;
                break;
            }
        }
        return status;
    }

    private LocalResourceAttribute getAttributeWithGivenAutomationId(
            SimulatorResource resource, int automationId) {
        LocalResourceAttribute targetAttribute = null;
        if (null != resource) {
            Map<String, LocalResourceAttribute> attributeMap = resource
                    .getResourceAttributesMap();
            if (null != attributeMap) {
                Set<String> attNameSet = attributeMap.keySet();
                Iterator<String> attNameItr = attNameSet.iterator();
                String attName;
                LocalResourceAttribute attribute;
                while (attNameItr.hasNext()) {
                    attName = attNameItr.next();
                    if (null != attName) {
                        attribute = attributeMap.get(attName);
                        if (null != attribute) {
                            if (attribute.isAutomationInProgress()
                                    && (attribute.getAutomationId() == automationId)) {
                                targetAttribute = attribute;
                                break;
                            }
                        }
                    }
                }
            }
        }
        return targetAttribute;
    }

    public boolean startResourceAutomationUIRequest(final String resourceURI) {
        if (null == resourceURI) {
            return false;
        }
        boolean status = false;
        SimulatorResource resource = getSimulatorResourceByURI(resourceURI);
        if (null != resource) {
            changeResourceLevelAutomationStatus(resource, true);

            // Invoke the native automation method
            SimulatorResourceServer resourceServer = resource
                    .getResourceServer();
            if (null != resourceServer) {
                // TODO: Temporarily handling the normal one-time automation for
                // resources
                int autoId = -1;
                try {
                    autoId = resourceServer.startResourceAutomation(
                            AutomationType.NORMAL, automationListener);
                } catch (SimulatorException e) {
                    Activator
                            .getDefault()
                            .getLogManager()
                            .log(Level.ERROR.ordinal(),
                                    new Date(),
                                    "[" + e.getClass().getSimpleName() + "]"
                                            + e.code().toString() + "-"
                                            + e.message());
                    autoId = -1;
                }
                if (-1 == autoId) {
                    // Automation request failed and hence status is being
                    // rolled back
                    changeResourceLevelAutomationStatus(resource, false);
                } else {
                    // Automation request accepted.
                    resource.setAutomationId(autoId);

                    // Notify the UI listeners in a different thread.
                    Thread notifyThread = new Thread() {
                        public void run() {
                            resourceAutomationStartedUINotification(resourceURI);
                        };
                    };
                    notifyThread.setPriority(Thread.MAX_PRIORITY);
                    notifyThread.start();

                    status = true;
                }
            }
        }
        return status;
    }

    public boolean stopResourceAutomationUIRequest(final String resourceURI) {
        SimulatorResource resource = getSimulatorResourceByURI(resourceURI);
        if (null == resource) {
            return false;
        }
        final int autoId = resource.getAutomationId();
        if (-1 == autoId) {
            return false;
        }
        SimulatorResourceServer resourceServer = resource.getResourceServer();
        if (null == resourceServer) {
            return false;
        }
        // Call native method
        try {
            resourceServer.stopAutomation(autoId);
        } catch (SimulatorException e) {
            Activator
                    .getDefault()
                    .getLogManager()
                    .log(Level.ERROR.ordinal(),
                            new Date(),
                            "[" + e.getClass().getSimpleName() + "]"
                                    + e.code().toString() + "-" + e.message());
            return false;
        }

        // Notify the UI Listeners. Invoke the automation complete callback.
        Thread stopThread = new Thread() {
            public void run() {
                automationListener.onAutomationComplete(resourceURI, autoId);
            }
        };
        stopThread.start();
        return true;
    }

    // Changes the automation state of the resource and its attributes
    private void changeResourceLevelAutomationStatus(
            SimulatorResource resource, boolean status) {

        Map<String, LocalResourceAttribute> attributeMap = resource
                .getResourceAttributesMap();
        if (null != attributeMap) {
            Set<String> attrNameSet = attributeMap.keySet();
            Iterator<String> attrNameItr = attrNameSet.iterator();
            String attrName;
            LocalResourceAttribute attribute;
            while (attrNameItr.hasNext()) {
                attrName = attrNameItr.next();
                attribute = attributeMap.get(attrName);
                if (null != attribute) {
                    attribute.setAutomationInProgress(status);
                }
            }
        }
        resource.setResourceAutomationInProgress(status);
    }

    public boolean isResourceAutomationStarted(String resourceURI) {
        boolean status = false;
        if (null == resourceURI) {
            return status;
        }

        SimulatorResource resource = getSimulatorResourceByURI(resourceURI);
        if (null != resource) {
            status = resource.isResourceAutomationInProgress();
        }
        return status;
    }

    public boolean isAttributeAutomationStarted(String resourceURI) {
        boolean status = false;
        if (null == resourceURI) {
            return status;
        }
        SimulatorResource resource = getSimulatorResourceByURI(resourceURI);
        if (null != resource) {
            status = resource.isAttributeAutomationInProgress();
        }
        return status;
    }

    public LocalResourceAttribute getAttributeByResourceURI(String uri,
            String attName) {
        if (null == uri || null == attName) {
            return null;
        }
        SimulatorResource resource = getSimulatorResourceByURI(uri);
        if (null == resource) {
            return null;
        }
        Map<String, LocalResourceAttribute> attMap = resource
                .getResourceAttributesMap();
        if (null == attMap) {
            return null;
        }
        return attMap.get(attName);
    }

    public void notifyObserverRequest(SimulatorResource res, int observerId) {
        if (null == res) {
            return;
        }
        SimulatorResourceServer server = res.getResourceServer();
        if (null == server) {
            return;
        }
        try {
            server.notifyObserver(observerId);
        } catch (SimulatorException e) {
            Activator
                    .getDefault()
                    .getLogManager()
                    .log(Level.ERROR.ordinal(),
                            new Date(),
                            "[" + e.getClass().getSimpleName() + "]"
                                    + e.code().toString() + "-" + e.message());
        }
    }

    public Image getImage(String resourceURI) {
        if (null == resourceURI) {
            return null;
        }
        SimulatorResource resource = getSimulatorResourceByURI(resourceURI);
        if (null == resource) {
            return null;
        }
        return Activator.getDefault().getImageRegistry()
                .get(resource.getResourceType());
    }

    public void shutdown() {
        threadHandle.interrupt();
    }
}
