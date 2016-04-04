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

package oic.simulator.clientcontroller.manager;

import java.net.URL;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;

import oic.simulator.clientcontroller.Activator;
import oic.simulator.clientcontroller.listener.IConfigurationUpload;
import oic.simulator.clientcontroller.listener.IFindResourceUIListener;
import oic.simulator.clientcontroller.listener.IGetUIListener;
import oic.simulator.clientcontroller.listener.IObserveUIListener;
import oic.simulator.clientcontroller.listener.IPostUIListener;
import oic.simulator.clientcontroller.listener.IPutUIListener;
import oic.simulator.clientcontroller.listener.IResourceSelectionChangedUIListener;
import oic.simulator.clientcontroller.listener.IVerificationUIListener;
import oic.simulator.clientcontroller.remoteresource.MetaProperty;
import oic.simulator.clientcontroller.remoteresource.PutPostAttributeModel;
import oic.simulator.clientcontroller.remoteresource.RemoteResource;
import oic.simulator.clientcontroller.remoteresource.RemoteResourceAttribute;
import oic.simulator.clientcontroller.utils.Constants;
import oic.simulator.clientcontroller.utils.Utility;

import org.eclipse.jface.resource.ImageDescriptor;
import org.eclipse.swt.graphics.Image;
import org.oic.simulator.ILogger.Level;
import org.oic.simulator.InvalidArgsException;
import org.oic.simulator.ResourceAttribute;
import org.oic.simulator.ResourceAttribute.Range;
import org.oic.simulator.ResourceAttribute.Type;
import org.oic.simulator.SimulatorException;
import org.oic.simulator.SimulatorManager;
import org.oic.simulator.SimulatorResourceModel;
import org.oic.simulator.clientcontroller.IFindResourceListener;
import org.oic.simulator.clientcontroller.IGetListener;
import org.oic.simulator.clientcontroller.IObserveListener;
import org.oic.simulator.clientcontroller.IPostListener;
import org.oic.simulator.clientcontroller.IPutListener;
import org.oic.simulator.clientcontroller.IVerificationListener;
import org.oic.simulator.clientcontroller.SimulatorObserveType;
import org.oic.simulator.clientcontroller.SimulatorRemoteResource;
import org.oic.simulator.clientcontroller.SimulatorVerificationType;

/**
 * This class acts as an interface between the simulator java SDK and the
 * various UI modules. It maintains all the details of resources and provides
 * other UI modules with the information required. It also handles responses for
 * find, GET, PUT, POST, Observe and automatic verification operations from
 * native layer and propagates those events to the registered UI listeners.
 */
public class ResourceManager {

    private Set<String>                               lastKnownSearchTypes;

    private RemoteResource                            currentResourceInSelection;

    private IFindResourceListener                     findResourceListener;
    private IGetListener                              getListener;
    private IPutListener                              putListener;
    private IPostListener                             postListener;
    private IObserveListener                          observeListener;
    private IVerificationListener                     verifyListener;

    private ResponseSynchronizerThread                synchronizerThread;

    private Thread                                    threadHandle;

    private List<IFindResourceUIListener>             findResourceUIListeners;
    private List<IResourceSelectionChangedUIListener> resourceSelectionChangedUIListeners;
    private List<IGetUIListener>                      getUIListeners;
    private List<IPutUIListener>                      putUIListeners;
    private List<IPostUIListener>                     postUIListeners;
    private List<IObserveUIListener>                  observeUIListeners;
    private List<IVerificationUIListener>             verificationUIListeners;
    private List<IConfigurationUpload>                configUploadUIListeners;

    // Map with Server ID as key and the complete object as the value
    private Map<String, RemoteResource>               resourceMap;
    private List<RemoteResource>                      favoriteResources;
    // Maintaining a list of resource URIs for favorite resources feature.
    private List<String>                              favoriteURIList;

    // Maintaining a list of observed resource URIs.
    private List<String>                              observedResourceURIList;

    public ResourceManager() {
        resourceMap = new HashMap<String, RemoteResource>();
        favoriteResources = new ArrayList<RemoteResource>();
        favoriteURIList = new ArrayList<String>();
        observedResourceURIList = new ArrayList<String>();
        findResourceUIListeners = new ArrayList<IFindResourceUIListener>();
        resourceSelectionChangedUIListeners = new ArrayList<IResourceSelectionChangedUIListener>();
        getUIListeners = new ArrayList<IGetUIListener>();
        putUIListeners = new ArrayList<IPutUIListener>();
        postUIListeners = new ArrayList<IPostUIListener>();
        observeUIListeners = new ArrayList<IObserveUIListener>();
        verificationUIListeners = new ArrayList<IVerificationUIListener>();
        configUploadUIListeners = new ArrayList<IConfigurationUpload>();

        findResourceListener = new IFindResourceListener() {

            @Override
            public void onResourceCallback(
                    final SimulatorRemoteResource resourceN) {
                synchronizerThread.addToQueue(new Runnable() {
                    @Override
                    public void run() {
                        System.out.println("onResourceCallback() entry");
                        if (null == resourceN) {
                            return;
                        }
                        // If resource already exist, then ignore it.
                        String uid = resourceN.getId();
                        if (null == uid) {
                            return;
                        }
                        boolean exist = isUidExist(uid);
                        if (exist) {
                            System.out.println("Duplicate resource found: ["
                                    + uid + "]");
                            return;
                        }

                        // Fetch the resource data
                        RemoteResource resource = fetchResourceDetails(resourceN);
                        if (null == resource) {
                            return;
                        }

                        resource.setResource(resourceN);

                        String uri = resource.getResourceURI();
                        if (null != uri) {
                            // Add resource to favorite list if it was in
                            // favorites list during find/refresh operation.
                            if (favoriteURIList.contains(uri)) {
                                addResourcetoFavorites(resource);
                            }
                            // Add resource to observed resources list if it was
                            // in observe list during find/refresh operation.
                            if (observedResourceURIList.contains(uri)) {
                                resource.setObserved(true);
                            }
                        }

                        // Add the resource in local data structure
                        addResourceDetails(resource);

                        // Notify the UI listener
                        newResourceFoundNotification(resource);

                        // Send an initial GET request to get the resource
                        // attributes
                        try {
                            resourceN.get(null, getListener);
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
                });
            }
        };

        getListener = new IGetListener() {
            @Override
            public void onGetCompleted(final String uid,
                    final SimulatorResourceModel resourceModelN) {
                synchronizerThread.addToQueue(new Runnable() {
                    @Override
                    public void run() {
                        // Handling the response which includes retrieving the
                        // attributes and updating the local model.
                        RemoteResource resource = handleResponse(uid,
                                resourceModelN);
                        if (null != resource) {
                            // Notify the UI listeners
                            getCompleteNotification(resource);
                        }
                    }
                });
            }

            @Override
            public void onGetFailed(Throwable th) {
                synchronizerThread.addToQueue(new Runnable() {
                    @Override
                    public void run() {
                    }
                });
            }
        };

        putListener = new IPutListener() {

            @Override
            public void onPutCompleted(final String uid,
                    final SimulatorResourceModel resourceModelN) {
                synchronizerThread.addToQueue(new Thread() {
                    @Override
                    public void run() {
                        // Handling the response which includes retrieving the
                        // attributes and updating the local model.
                        RemoteResource resource = handleResponse(uid,
                                resourceModelN);
                        if (null != resource) {
                            // Notify the UI listeners
                            putCompleteNotification(resource);
                        }
                    }
                });
            }

            @Override
            public void onPutFailed(Throwable th) {
                synchronizerThread.addToQueue(new Runnable() {
                    @Override
                    public void run() {
                    }
                });
            }
        };

        postListener = new IPostListener() {
            @Override
            public void onPostCompleted(final String uid,
                    final SimulatorResourceModel resourceModelN) {
                synchronizerThread.addToQueue(new Runnable() {
                    @Override
                    public void run() {
                        // Handling the response which includes retrieving the
                        // attributes and updating the local model.
                        RemoteResource resource = handleResponse(uid,
                                resourceModelN);
                        if (null != resource) {
                            // Notify the UI listeners
                            postCompleteNotification(resource);
                        }
                    }
                });
            }

            @Override
            public void onPostFailed(Throwable th) {
                synchronizerThread.addToQueue(new Runnable() {
                    @Override
                    public void run() {
                    }
                });
            }
        };

        observeListener = new IObserveListener() {

            @Override
            public void onObserveCompleted(final String uid,
                    final SimulatorResourceModel resourceModelN, final int seq) {
                System.out.println("ResourceManager: onObserveCallback()");
                synchronizerThread.addToQueue(new Runnable() {
                    @Override
                    public void run() {
                        // Handling the response which includes retrieving the
                        // attributes and updating the local model.
                        RemoteResource resource = handleResponse(uid,
                                resourceModelN);
                        if (null != resource) {
                            // Notify the UI listeners
                            observeCompleteNotification(resource);
                        }
                    }
                });
            }

            @Override
            public void onObserveFailed(Throwable th) {
                // TODO Auto-generated method stub
            }
        };

        verifyListener = new IVerificationListener() {

            @Override
            public void onVerificationStarted(final String uid, final int autoId) {
                System.out.println("onVefificationStarted: " + autoId);
                synchronizerThread.addToQueue(new Runnable() {
                    @Override
                    public void run() {
                        RemoteResource resource = getResource(uid);
                        if (null == resource) {
                            return;
                        }
                        // Update the automation status.
                        resource.updateAutomationStatus(autoId, true);

                        int autoType = resource.getAutomationtype(autoId);

                        // Notify the listeners.
                        verificationStartedNotification(resource, autoType);
                    }
                });
            }

            @Override
            public void onVerificationCompleted(final String uid,
                    final int autoId) {
                System.out.println("onVefificationCompleted: " + autoId);
                synchronizerThread.addToQueue(new Runnable() {
                    @Override
                    public void run() {
                        RemoteResource resource = getResource(uid);
                        if (null == resource) {
                            return;
                        }
                        // Update the automation status.
                        resource.updateAutomationStatus(autoId, false);

                        int autoType = resource.getAutomationtype(autoId);

                        // Notify the listeners.
                        verificationCompletedNotification(resource, autoType);
                    }
                });
            }

            @Override
            public void onVerificationAborted(final String uid, final int autoId) {
                System.out.println("onVefificationAborted: " + autoId);
                synchronizerThread.addToQueue(new Runnable() {
                    @Override
                    public void run() {
                        RemoteResource resource = getResource(uid);
                        if (null == resource) {
                            return;
                        }
                        // Update the automation status.
                        resource.updateAutomationStatus(autoId, false);

                        int autoType = resource.getAutomationtype(autoId);

                        // Notify the listeners.
                        verificationAbortedNotification(resource, autoType);
                    }
                });
            }
        };

        synchronizerThread = new ResponseSynchronizerThread();
        threadHandle = new Thread(synchronizerThread);
        threadHandle.setName("Simulator Client Controller Event Queue");
        threadHandle.start();
    }

    private RemoteResource handleResponse(String uid,
            SimulatorResourceModel resourceModelN) {
        if (null == uid || null == resourceModelN) {
            return null;
        }

        // Update the local model
        RemoteResource resource;
        resource = getResource(uid);
        if (null == resource) {
            return null;
        }

        resource.setResourceModel(resourceModelN);
        Map<String, RemoteResourceAttribute> attributeMap = fetchResourceAttributesFromModel(resourceModelN);

        // TODO: For debugging
        if (null != attributeMap) {
            RemoteResourceAttribute.printAttributes(attributeMap);
            System.out.println("Attributes found: ");
            System.out.println("No of attributes: " + attributeMap.size());

            resource.setResourceAttributesMap(attributeMap);
        }
        return resource;
    }

    private static class ResponseSynchronizerThread implements Runnable {

        LinkedList<Runnable> responseQueue = new LinkedList<Runnable>();

        @Override
        public void run() {
            while (!Thread.interrupted()) {
                synchronized (this) {
                    try {
                        while (responseQueue.isEmpty()) {
                            this.wait();
                            break;
                        }
                    } catch (InterruptedException e) {
                        return;
                    }
                }

                Runnable thread;
                synchronized (this) {
                    thread = responseQueue.pop();
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
                responseQueue.add(event);
                this.notify();
            }
        }
    }

    public void addResourceSelectionChangedUIListener(
            IResourceSelectionChangedUIListener resourceSelectionChangedUIListener) {
        synchronized (resourceSelectionChangedUIListeners) {
            resourceSelectionChangedUIListeners
                    .add(resourceSelectionChangedUIListener);
        }
    }

    public void addGetUIListener(IGetUIListener getUIListener) {
        synchronized (getUIListeners) {
            getUIListeners.add(getUIListener);
        }
    }

    public void addPutUIListener(IPutUIListener putUIListener) {
        synchronized (putUIListeners) {
            putUIListeners.add(putUIListener);
        }
    }

    public void addPostUIListener(IPostUIListener postUIListener) {
        synchronized (postUIListeners) {
            postUIListeners.add(postUIListener);
        }
    }

    public void addObserveUIListener(IObserveUIListener observeUIListener) {
        synchronized (observeUIListeners) {
            observeUIListeners.add(observeUIListener);
        }
    }

    public void addVerificationUIListener(
            IVerificationUIListener verificationUIListener) {
        synchronized (verificationUIListeners) {
            verificationUIListeners.add(verificationUIListener);
        }
    }

    public void addConfigUploadUIListener(IConfigurationUpload configListener) {
        synchronized (configUploadUIListeners) {
            configUploadUIListeners.add(configListener);
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

    public void removeGetUIListener(IGetUIListener getUIListener) {
        synchronized (getUIListeners) {
            getUIListeners.remove(getUIListener);
        }
    }

    public void removePutUIListener(IPutUIListener putUIListener) {
        synchronized (putUIListeners) {
            putUIListeners.remove(putUIListener);
        }
    }

    public void removePostUIListener(IPostUIListener postUIListener) {
        synchronized (postUIListeners) {
            postUIListeners.remove(postUIListener);
        }
    }

    public void removeObserveUIListener(IObserveUIListener observeUIListener) {
        synchronized (observeUIListeners) {
            observeUIListeners.remove(observeUIListener);
        }
    }

    public void removeVerificationUIListener(
            IVerificationUIListener verificationUIListener) {
        synchronized (verificationUIListeners) {
            verificationUIListeners.remove(verificationUIListener);
        }
    }

    public void removeConfigUploadUIListener(IConfigurationUpload configListener) {
        synchronized (configUploadUIListeners) {
            configUploadUIListeners.remove(configListener);
        }
    }

    public void addResourcetoFavorites(RemoteResource resource) {
        if (null == resource) {
            return;
        }
        resource.setFavorite(true);
        synchronized (favoriteResources) {
            favoriteResources.add(resource);
            favoriteURIList.add(resource.getResourceURI());
        }
    }

    public void removeResourceFromFavorites(RemoteResource resource) {
        if (null == resource) {
            return;
        }
        resource.setFavorite(false);
        synchronized (favoriteResources) {
            favoriteResources.remove(resource);
        }
    }

    public void removeResourceURIFromFavorites(RemoteResource resource) {
        if (null == resource) {
            return;
        }
        synchronized (favoriteURIList) {
            favoriteURIList.remove(resource.getResourceURI());
        }
    }

    public void addObservedResourceURI(String resourceURI) {
        synchronized (observedResourceURIList) {
            observedResourceURIList.add(resourceURI);
        }
    }

    public void removeObservedResourceURI(String resourceURI) {
        synchronized (observedResourceURIList) {
            observedResourceURIList.remove(resourceURI);
        }
    }

    public boolean isResourceObserved(String resourceURI) {
        boolean observed;
        synchronized (observedResourceURIList) {
            observed = observedResourceURIList.contains(resourceURI);
        }
        return observed;
    }

    public synchronized RemoteResource getCurrentResourceInSelection() {
        return currentResourceInSelection;
    }

    public synchronized void setCurrentResourceInSelection(
            RemoteResource resource) {
        this.currentResourceInSelection = resource;
    }

    private void addResourceDetails(RemoteResource remoteResource) {
        if (null != remoteResource) {
            synchronized (resourceMap) {
                resourceMap.put(remoteResource.getuId(), remoteResource);
            }
        }
    }

    public void addFindresourceUIListener(IFindResourceUIListener listener) {
        if (null == listener) {
            return;
        }
        synchronized (findResourceUIListeners) {
            findResourceUIListeners.add(listener);
        }
    }

    public void removeFindresourceUIListener(IFindResourceUIListener listener) {
        if (null == listener) {
            return;
        }
        synchronized (findResourceUIListeners) {
            findResourceUIListeners.remove(listener);
        }
    }

    private RemoteResource fetchResourceDetails(
            SimulatorRemoteResource remoteResourceN) {
        if (null == remoteResourceN) {
            return null;
        }
        RemoteResource remoteResource = new RemoteResource();
        remoteResource.setuId(remoteResourceN.getId());
        remoteResource.setResourceURI(remoteResourceN.getUri());
        remoteResource.setHost(remoteResourceN.getHost());
        remoteResource.setResourceTypes(remoteResourceN.getResourceTypes());
        remoteResource.setResourceInterfaces(remoteResourceN
                .getResourceInterfaces());
        remoteResource.setConnectivityType(remoteResourceN
                .getConnectivityType());
        remoteResource.setObservable(remoteResourceN.getIsObservable());
        return remoteResource;
    }

    private boolean isUidExist(String uid) {
        boolean exist;
        synchronized (resourceMap) {
            exist = resourceMap.containsKey(uid);
        }
        return exist;
    }

    private RemoteResource getResource(String uid) {
        if (null == uid) {
            return null;
        }
        RemoteResource resource;
        synchronized (resourceMap) {
            resource = resourceMap.get(uid);
        }
        return resource;
    }

    private Map<String, RemoteResourceAttribute> fetchResourceAttributesFromModel(
            SimulatorResourceModel resourceModelN) {
        Map<String, RemoteResourceAttribute> resourceAttributeMap = null;
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
                resourceAttributeMap = new HashMap<String, RemoteResourceAttribute>();

                Set<String> attNameSet = attributeMapN.keySet();
                String attName;
                Object attValueObj;
                ResourceAttribute attributeN;
                RemoteResourceAttribute attribute;
                Iterator<String> attNameItr = attNameSet.iterator();
                while (attNameItr.hasNext()) {
                    attName = attNameItr.next();
                    attributeN = attributeMapN.get(attName);
                    if (null != attributeN) {
                        attribute = new RemoteResourceAttribute();
                        attribute.setResourceAttribute(attributeN);
                        attribute.setAttributeName(attName);

                        attValueObj = attributeN.getValue();
                        if (null != attValueObj) {
                            attribute.setAttributeValue(attValueObj);
                        }

                        // Set the attribute type
                        attribute.setAttValBaseType(attributeN.getBaseType());
                        attribute.setAttValType(attributeN.getType());

                        // Set the range and allowed values
                        Range range = attributeN.getRange();
                        if (null != range) {
                            attribute.setMinValue(range.getMin());
                            attribute.setMaxValue(range.getMax());
                        } else {
                            Object[] values = attributeN.getAllowedValues();
                            if (null != values && values.length > 0) {
                                List<Object> valueList = new ArrayList<Object>();
                                for (Object obj : values) {
                                    valueList.add(obj);
                                }
                                attribute.setAllowedValues(valueList);
                            }
                            /*
                             * Type baseType = attribute.getAttValBaseType();
                             *
                             * if(baseType == Type.INT) { //int[] values =
                             * attributeN.getAllowedValues();
                             * attribute.setAllowedValues
                             * (attributeN.getAllowedValues()); } else
                             * if(baseType == Type.DOUBLE) { double[] values =
                             * attributeN.getAllowedValues();
                             * attribute.setAllowedValues
                             * (Utility.converArrayToList(values)); } else
                             * if(baseType == Type.BOOL) { //boolean[] values =
                             * attributeN.getAllowedValues(); List<Object> obj =
                             * new ArrayList<Object>(); obj.add(true);
                             * obj.add(false); attribute.setAllowedValues(obj);
                             * } else if(baseType == Type.STRING) { String[]
                             * values = attributeN.getAllowedValues();
                             * attribute.
                             * setAllowedValues(Utility.converArrayToList
                             * (values)); }
                             */
                        }
                        resourceAttributeMap.put(attName, attribute);
                    }
                }
            }
        }
        return resourceAttributeMap;
    }

    private void newResourceFoundNotification(RemoteResource resource) {
        synchronized (findResourceUIListeners) {
            if (findResourceUIListeners.size() > 0) {
                IFindResourceUIListener listener;
                Iterator<IFindResourceUIListener> listenerItr = findResourceUIListeners
                        .iterator();
                while (listenerItr.hasNext()) {
                    listener = listenerItr.next();
                    if (null != listener) {
                        listener.onNewResourceFound(resource);
                    }
                }
            }
        }
    }

    private void resourceSelectionChangedUINotification(RemoteResource resource) {
        synchronized (resourceSelectionChangedUIListeners) {
            if (resourceSelectionChangedUIListeners.size() > 0) {
                IResourceSelectionChangedUIListener listener;
                Iterator<IResourceSelectionChangedUIListener> listenerItr = resourceSelectionChangedUIListeners
                        .iterator();
                while (listenerItr.hasNext()) {
                    listener = listenerItr.next();
                    if (null != listener) {
                        listener.onResourceSelectionChange(resource);
                    }
                }
            }
        }
    }

    private void getCompleteNotification(RemoteResource resource) {
        synchronized (getUIListeners) {
            if (getUIListeners.size() > 0) {
                IGetUIListener listener;
                Iterator<IGetUIListener> listenerItr = getUIListeners
                        .iterator();
                while (listenerItr.hasNext()) {
                    listener = listenerItr.next();
                    if (null != listener) {
                        listener.onGetCompleted(resource);
                    }
                }
            }
        }
    }

    private void putCompleteNotification(RemoteResource resource) {
        synchronized (putUIListeners) {
            if (putUIListeners.size() > 0) {
                IPutUIListener listener;
                Iterator<IPutUIListener> listenerItr = putUIListeners
                        .iterator();
                while (listenerItr.hasNext()) {
                    listener = listenerItr.next();
                    if (null != listener) {
                        listener.onPutCompleted(resource);
                    }
                }
            }
        }
    }

    private void postCompleteNotification(RemoteResource resource) {
        synchronized (postUIListeners) {
            if (postUIListeners.size() > 0) {
                IPostUIListener listener;
                Iterator<IPostUIListener> listenerItr = postUIListeners
                        .iterator();
                while (listenerItr.hasNext()) {
                    listener = listenerItr.next();
                    if (null != listener) {
                        listener.onPostCompleted(resource);
                    }
                }
            }
        }
    }

    private void observeCompleteNotification(RemoteResource resource) {
        synchronized (observeUIListeners) {
            if (observeUIListeners.size() > 0) {
                IObserveUIListener listener;
                Iterator<IObserveUIListener> listenerItr = observeUIListeners
                        .iterator();
                while (listenerItr.hasNext()) {
                    listener = listenerItr.next();
                    if (null != listener) {
                        listener.onObserveCompleted(resource);
                    }
                }
            }
        }
    }

    private void verificationStartedNotification(RemoteResource resource,
            int autoType) {
        synchronized (verificationUIListeners) {
            if (verificationUIListeners.size() > 0) {
                IVerificationUIListener listener;
                Iterator<IVerificationUIListener> listenerItr = verificationUIListeners
                        .iterator();
                while (listenerItr.hasNext()) {
                    listener = listenerItr.next();
                    if (null != listener) {
                        listener.onVerificationStarted(resource, autoType);
                    }
                }
            }
        }
    }

    private void verificationAbortedNotification(RemoteResource resource,
            int autoType) {
        synchronized (verificationUIListeners) {
            if (verificationUIListeners.size() > 0) {
                IVerificationUIListener listener;
                Iterator<IVerificationUIListener> listenerItr = verificationUIListeners
                        .iterator();
                while (listenerItr.hasNext()) {
                    listener = listenerItr.next();
                    if (null != listener) {
                        listener.onVerificationAborted(resource, autoType);
                    }
                }
            }
        }
    }

    private void verificationCompletedNotification(RemoteResource resource,
            int autoType) {
        synchronized (verificationUIListeners) {
            if (verificationUIListeners.size() > 0) {
                IVerificationUIListener listener;
                Iterator<IVerificationUIListener> listenerItr = verificationUIListeners
                        .iterator();
                while (listenerItr.hasNext()) {
                    listener = listenerItr.next();
                    if (null != listener) {
                        listener.onVerificationCompleted(resource, autoType);
                    }
                }
            }
        }
    }

    private void configUploadedNotification(RemoteResource resource) {
        synchronized (configUploadUIListeners) {
            if (configUploadUIListeners.size() > 0) {
                IConfigurationUpload listener;
                Iterator<IConfigurationUpload> listenerItr = configUploadUIListeners
                        .iterator();
                while (listenerItr.hasNext()) {
                    listener = listenerItr.next();
                    if (null != listener) {
                        listener.onConfigurationUploaded(resource);
                    }
                }
            }
        }
    }

    // TODO: Temporarily used to display the resource in the UI
    public List<String> getURIList() {
        List<String> list = new ArrayList<String>();
        synchronized (resourceMap) {
            /*
             * Set<String> idSet = resourceMap.keySet(); Iterator<String> idItr
             * = idSet.iterator(); String sId; RemoteResource resource;
             * while(idItr.hasNext()) { sId = idItr.next(); resource =
             * resourceMap.get(sId); if(null == resource) { continue; }
             * list.add(resource.getResourceURI()); }
             */
            Set<String> uriSet = resourceMap.keySet();
            Iterator<String> uriItr = uriSet.iterator();
            String uri;
            while (uriItr.hasNext()) {
                uri = uriItr.next();
                if (null != uri) {
                    list.add(uri);
                }
            }

            // Sort the types
            Collections.sort(list);
        }
        return list;
    }

    public synchronized Set<String> getLastKnownSearchTypes() {
        return lastKnownSearchTypes;
    }

    public synchronized void setLastKnownSearchTypes(
            Set<String> lastKnownSearchTypes) {
        this.lastKnownSearchTypes = lastKnownSearchTypes;
    }

    public boolean findResourceRequest(Set<String> searchTypes) {
        if (null == searchTypes || searchTypes.size() < 1) {
            return false;
        }
        boolean result = false;
        Iterator<String> searchItr = searchTypes.iterator();
        String rType;
        while (searchItr.hasNext()) {
            rType = searchItr.next();
            try {
                SimulatorManager.findResource(rType, findResourceListener);
                result = true;
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
        return result;
    }

    public void deleteResources(final Set<String> searchTypes) {
        if (null == searchTypes || searchTypes.size() < 1) {
            return;
        }
        new Thread() {
            public void run() {
                Iterator<String> typeItr = searchTypes.iterator();
                String resType;
                while (typeItr.hasNext()) {
                    resType = typeItr.next();
                    deleteResourcesByType(resType);

                    // Change the current resource in selection
                    updateCurrentResourceInSelection(searchTypes);
                }
            }
        }.start();
    }

    private void updateCurrentResourceInSelection(Set<String> searchTypes) {
        if (null == searchTypes || searchTypes.size() < 1) {
            return;
        }
        RemoteResource resourceInSelection = getCurrentResourceInSelection();
        if (null == resourceInSelection) {
            return;
        }
        List<String> typesOfSelection = resourceInSelection.getResourceTypes();
        if (null == typesOfSelection || typesOfSelection.size() < 1) {
            return;
        }
        Iterator<String> itr = typesOfSelection.iterator();
        String type;
        while (itr.hasNext()) {
            type = itr.next();
            if (searchTypes.contains(type)) {
                setCurrentResourceInSelection(null);
                resourceSelectionChangedUINotification(null);
                break;
            }
        }
    }

    private void deleteResourcesByType(String resourceType) {
        if (null == resourceType) {
            return;
        }
        synchronized (resourceMap) {
            Set<String> keySet = resourceMap.keySet();
            if (null == keySet) {
                return;
            }
            Iterator<String> keyItr = keySet.iterator();
            String uId;
            RemoteResource resource;
            boolean exist;
            List<String> types;
            while (keyItr.hasNext()) {
                uId = keyItr.next();
                resource = resourceMap.get(uId);
                if (null == resource) {
                    continue;
                }
                types = resource.getResourceTypes();
                if (null != types) {
                    exist = types.contains(resourceType);
                    if (exist) {
                        // Remove the resource
                        keyItr.remove();
                        // Remove the resource from favorites list.
                        removeResourceFromFavorites(resource);
                    }
                }
            }
        }
    }

    public void resourceSelectionChanged(final RemoteResource resource) {
        new Thread() {
            @Override
            public void run() {
                setCurrentResourceInSelection(resource);
                // Notify all observers for resource selection change event
                resourceSelectionChangedUINotification(resource);
            }
        }.start();
    }

    public List<MetaProperty> getMetaProperties(RemoteResource resource) {
        if (null != resource) {
            String propName;
            String propValue;

            List<MetaProperty> metaPropertyList = new ArrayList<MetaProperty>();

            for (int index = 0; index < Constants.META_PROPERTY_COUNT; index++) {
                propName = Constants.META_PROPERTIES[index];
                if (propName.equals(Constants.RESOURCE_URI)) {
                    propValue = resource.getResourceURI();
                } else if (propName.equals(Constants.CONNECTIVITY_TYPE)) {
                    propValue = resource.getConnectivityType().toString();
                } else if (propName.equals(Constants.OBSERVABLE)) {
                    propValue = Utility.getObservableInString(resource
                            .isObservable());
                    // see in UI
                } else if (propName.equals(Constants.RESOURCE_TYPES)) {
                    List<String> types = resource.getResourceTypes();
                    if (null != types) {
                        propValue = types.toString();
                    } else {
                        propValue = Constants.NOT_AVAILABLE;
                    }
                } else if (propName.equals(Constants.RESOURCE_INTERFACES)) {
                    List<String> interfaces = resource.getResourceInterfaces();
                    if (null != interfaces) {
                        propValue = interfaces.toString();
                    } else {
                        propValue = Constants.NOT_AVAILABLE;
                    }
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

    public Map<String, Boolean> getAutomationStatus(RemoteResource resource) {
        if (null == resource) {
            return null;
        }
        Map<String, Boolean> autoStatus = new HashMap<String, Boolean>();
        autoStatus.put(Constants.GET, resource.isGetAutomtnInProgress());
        autoStatus.put(Constants.PUT, resource.isPutAutomtnInProgress());
        autoStatus.put(Constants.POST, resource.isPostAutomtnInProgress());
        return autoStatus;
    }

    public Map<String, String> getDummyAttributes() {
        Map<String, String> attributes = new HashMap<String, String>();
        attributes.put("intensity", "1");
        attributes.put("power", "off");
        return attributes;
    }

    public List<RemoteResource> getResourceList() {
        List<RemoteResource> resourceList = new ArrayList<RemoteResource>();
        synchronized (resourceMap) {
            Set<String> idSet = resourceMap.keySet();
            Iterator<String> idItr = idSet.iterator();
            RemoteResource resource;
            while (idItr.hasNext()) {
                resource = resourceMap.get(idItr.next());
                if (null != resource) {
                    resourceList.add(resource);
                }
            }
        }
        // Sort the list
        Collections.sort(resourceList, new Comparator<RemoteResource>() {
            public int compare(RemoteResource res1, RemoteResource res2) {
                String s1 = res1.getResourceURI();
                String s2 = res2.getResourceURI();

                String s1Part = s1.replaceAll("\\d", "");
                String s2Part = s2.replaceAll("\\d", "");

                if (s1Part.equalsIgnoreCase(s2Part)) {
                    return extractInt(s1) - extractInt(s2);
                }
                return s1.compareTo(s2);
            }

            int extractInt(String s) {
                String num = s.replaceAll("\\D", "");
                // return 0 if no digits found
                return num.isEmpty() ? 0 : Integer.parseInt(num);
            }
        });

        return resourceList;
    }

    public List<RemoteResource> getFavResourceList() {
        List<RemoteResource> resourceList;
        synchronized (favoriteResources) {
            resourceList = new ArrayList<RemoteResource>(favoriteResources);
        }
        return resourceList;
    }

    public String getAttributeValue(RemoteResource res, String attName) {
        if (null == res || null == attName) {
            return null;
        }
        return res.getAttributeValue(attName);
    }

    public void sendGetRequest(RemoteResource resource) {
        if (null == resource) {
            return;
        }
        SimulatorRemoteResource resourceN = resource.getResource();
        if (null == resourceN) {
            return;
        }
        try {
            resourceN.get(null, getListener);
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

    public void sendPutRequest(RemoteResource resource,
            List<PutPostAttributeModel> putPostModelList) {
        System.out.println(putPostModelList);
        System.out.println("ResourceManager: sendPutRequest");
        if (null == resource) {
            return;
        }
        System.out.println("ResourceManager: resource not null");
        SimulatorRemoteResource resourceN = resource.getResource();
        if (null == resourceN) {
            return;
        }
        System.out.println("ResourceManager: Native resource not null");
        Map<String, RemoteResourceAttribute> attMap = resource
                .getResourceAttributesMap();
        if (null == attMap || attMap.size() < 1) {
            return;
        }
        System.out.println("ResourceManager: attrubutes obtained");
        SimulatorResourceModel resourceModel = getUpdatedResourceModel(attMap,
                putPostModelList);
        System.out.println("ResourceModel exist?:" + (resourceModel != null));
        try {
            resourceN.put(resourceModel, null, putListener);
        } catch (SimulatorException e) {
            Activator
                    .getDefault()
                    .getLogManager()
                    .log(Level.ERROR.ordinal(),
                            new Date(),
                            "[" + e.getClass().getSimpleName() + "]"
                                    + e.code().toString() + "-" + e.message());
        }
        System.out.println("ResourceManager: called native put");
    }

    public void sendPostRequest(RemoteResource resource,
            List<PutPostAttributeModel> putPostModelList) {
        System.out.println(putPostModelList);
        if (null == resource) {
            return;
        }
        SimulatorRemoteResource resourceN = resource.getResource();
        if (null == resourceN) {
            return;
        }
        Map<String, RemoteResourceAttribute> attMap = resource
                .getResourceAttributesMap();
        if (null == attMap || attMap.size() < 1) {
            return;
        }
        // Filter out the attributes whose modification status is true.
        Iterator<PutPostAttributeModel> itr = putPostModelList.iterator();
        PutPostAttributeModel model;
        while (itr.hasNext()) {
            model = itr.next();
            if (!model.isModified()) {
                itr.remove();
            }
        }
        SimulatorResourceModel resourceModel = getUpdatedResourceModel(attMap,
                putPostModelList);
        try {
            resourceN.post(resourceModel, null, postListener);
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

    private SimulatorResourceModel getUpdatedResourceModel(
            Map<String, RemoteResourceAttribute> attMap,
            List<PutPostAttributeModel> putPostModelList) {
        String attName;
        SimulatorResourceModel resourceModel = new SimulatorResourceModel();
        PutPostAttributeModel model;
        RemoteResourceAttribute attribute;
        Type attType;
        Iterator<PutPostAttributeModel> itr = putPostModelList.iterator();
        while (itr.hasNext()) {
            model = itr.next();
            attName = model.getAttName();
            attribute = attMap.get(attName);
            if (null == attribute) {
                continue;
            }
            attType = attribute.getAttValBaseType();
            if (attType == Type.INT) {
                int attValue;
                try {
                    attValue = Integer.parseInt(model.getAttValue());
                    resourceModel.addAttributeInt(attName, attValue);
                } catch (NumberFormatException e) {
                    Activator
                            .getDefault()
                            .getLogManager()
                            .log(Level.ERROR.ordinal(), new Date(),
                                    e.getMessage());
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
            } else if (attType == Type.DOUBLE) {
                double attValue;
                try {
                    attValue = Double.parseDouble(model.getAttValue());
                    resourceModel.addAttributeDouble(attName, attValue);
                } catch (NumberFormatException e) {
                    Activator
                            .getDefault()
                            .getLogManager()
                            .log(Level.ERROR.ordinal(), new Date(),
                                    e.getMessage());
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
            } else if (attType == Type.BOOL) {
                boolean attValue;
                attValue = Boolean.parseBoolean(model.getAttValue());
                try {
                    resourceModel.addAttributeBoolean(attName, attValue);
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
            } else if (attType == Type.STRING) {
                String attValue;
                attValue = model.getAttValue();
                try {
                    resourceModel.addAttributeString(attName, attValue);
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
        return resourceModel;
    }

    public void sendObserveRequest(RemoteResource resource) {
        System.out.println("sendObserverRequest() entry");
        if (null == resource) {
            return;
        }
        System.out.println("Resource is null:" + (resource == null));
        resource.setObserved(true);
        SimulatorRemoteResource resourceN = resource.getResource();
        if (null == resourceN) {
            return;
        }
        try {
            resourceN.startObserve(SimulatorObserveType.OBSERVE, null,
                    observeListener);
            // Add observed resource URI to show the proper status after every
            // find/refresh operations.
            addObservedResourceURI(resource.getResourceURI());
        } catch (SimulatorException e) {
            Activator
                    .getDefault()
                    .getLogManager()
                    .log(Level.ERROR.ordinal(),
                            new Date(),
                            "[" + e.getClass().getSimpleName() + "]"
                                    + e.code().toString() + "-" + e.message());
        }
        System.out.println("Observer called.");
    }

    public void sendCancelObserveRequest(RemoteResource resource) {
        if (null == resource) {
            return;
        }
        resource.setObserved(false);
        SimulatorRemoteResource resourceN = resource.getResource();
        if (null == resourceN) {
            return;
        }
        try {
            resourceN.stopObserve();
            // Remove observed resource URI to show the proper status after
            // every find/refresh operations.
            removeObservedResourceURI(resource.getResourceURI());
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

    public void startAutomationRequest(int reqType, RemoteResource resource) {
        if (null == resource) {
            return;
        }
        SimulatorRemoteResource resourceN = resource.getResource();
        if (null == resourceN) {
            return;
        }
        SimulatorVerificationType type = SimulatorVerificationType
                .getVerificationType(reqType);
        if (null == type) {
            return;
        }
        System.out.println("Before calling startVerification: " + reqType);
        int autoId;
        try {
            autoId = resourceN.startVerification(type, verifyListener);
            System.out.println("After calling startVerification: " + autoId);
            if (autoId != -1) {
                if (reqType == Constants.GET_AUTOMATION_INDEX) {
                    // resource.setGetAutomtnInProgress(true);
                    resource.setGetAutomtnId(autoId);
                } else if (reqType == Constants.PUT_AUTOMATION_INDEX) {
                    // resource.setPutAutomtnInProgress(true);
                    resource.setPutAutomtnId(autoId);
                } else {// if(reqType == Constants.POST_AUTOMATION_INDEX) {
                        // resource.setPostAutomtnInProgress(true);
                    resource.setPostAutomtnId(autoId);
                }
            }
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

    public void stopAutomationRequest(int reqType, RemoteResource resource) {
        if (null == resource) {
            return;
        }
        SimulatorRemoteResource resourceN = resource.getResource();
        if (null == resourceN) {
            return;
        }
        int autoId;
        if (reqType == Constants.GET_AUTOMATION_INDEX) {
            resource.setGetAutomtnInProgress(false);
            autoId = resource.getGetAutomtnId();
        } else if (reqType == Constants.PUT_AUTOMATION_INDEX) {
            resource.setPutAutomtnInProgress(false);
            autoId = resource.getPutAutomtnId();
        } else {// if(reqType == Constants.POST_AUTOMATION_INDEX) {
            resource.setPostAutomtnInProgress(false);
            autoId = resource.getPostAutomtnId();
        }
        try {
            resourceN.stopVerification(autoId);
        } catch (InvalidArgsException e) {
            Activator
            .getDefault()
            .getLogManager()
            .log(Level.ERROR.ordinal(),
                    new Date(),
                    "[" + e.getClass().getSimpleName() + "]"
                            + e.code().toString() + "-"
                            + e.message());
            return;
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

    public void setConfigFilePath(RemoteResource resource, String configFilePath) {
        if (null == resource) {
            return;
        }
        SimulatorRemoteResource resourceN = resource.getResource();
        if (null == resourceN) {
            return;
        }
        try {
            resourceN.setConfigInfo(configFilePath);
        } catch (SimulatorException e) {
            Activator
                    .getDefault()
                    .getLogManager()
                    .log(Level.ERROR.ordinal(),
                            new Date(),
                            "[" + e.getClass().getSimpleName() + "]"
                                    + e.code().toString() + "-" + e.message());
            return;
        }
        // Update the status
        resource.setConfigUploaded(true);

        // Notify the UI listeners
        configUploadedNotification(resource);
    }

    public Image getImage(String resourceURI) {
        if (null == resourceURI) {
            return null;
        }
        URL url = Activator.getDefault().getBundle()
                .getEntry(getImageURL(resourceURI));
        if (null == url) {
            return null;
        }
        return ImageDescriptor.createFromURL(url).createImage();
    }

    private String getImageURL(String resourceURI) {
        // TODO: Hard-coding the image file name temporarily.
        // It will be included in a separate class which manages all image
        // resources
        return "/icons/light_16x16.png";
    }

    public void shutdown() {
        // TODO: To be implemented for clean-up activities.
    }
}
