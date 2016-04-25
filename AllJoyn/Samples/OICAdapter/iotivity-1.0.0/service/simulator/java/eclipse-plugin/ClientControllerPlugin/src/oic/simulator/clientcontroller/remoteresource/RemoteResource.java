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

package oic.simulator.clientcontroller.remoteresource;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import oic.simulator.clientcontroller.utils.Constants;

import org.oic.simulator.SimulatorResourceModel;
import org.oic.simulator.clientcontroller.SimulatorConnectivityType;
import org.oic.simulator.clientcontroller.SimulatorRemoteResource;

/**
 * This class represents a remote resource. It maintains all the necessary
 * information about the resource.
 */
public class RemoteResource {
    private String                               uId;
    private String                               resourceURI;
    private String                               host;
    private LinkedList<String>                   resourceTypes;
    private LinkedList<String>                   resourceInterfaces;
    private SimulatorConnectivityType            connectivityType;
    private boolean                              isObservable;

    private boolean                              observed;

    // Native object references
    private SimulatorRemoteResource              resourceN;
    private SimulatorResourceModel               resourceModel;
    private Map<String, RemoteResourceAttribute> resourceAttributesMap;

    private boolean                              configUploaded;

    private boolean                              getAutomtnInProgress;
    private boolean                              putAutomtnInProgress;
    private boolean                              postAutomtnInProgress;

    private int                                  getAutomtnId;
    private int                                  putAutomtnId;
    private int                                  postAutomtnId;

    private boolean                              isFavorite;

    public SimulatorResourceModel getResourceModel() {
        return resourceModel;
    }

    public void setResourceModel(SimulatorResourceModel resourceModel) {
        this.resourceModel = resourceModel;
    }

    public Map<String, RemoteResourceAttribute> getResourceAttributesMap() {
        return resourceAttributesMap;
    }

    public void setResourceAttributesMap(
            Map<String, RemoteResourceAttribute> resourceAttributesMap) {
        this.resourceAttributesMap = resourceAttributesMap;
    }

    public int getGetAutomtnId() {
        return getAutomtnId;
    }

    public void setGetAutomtnId(int getAutomtnId) {
        this.getAutomtnId = getAutomtnId;
    }

    public int getPutAutomtnId() {
        return putAutomtnId;
    }

    public void setPutAutomtnId(int putAutomtnId) {
        this.putAutomtnId = putAutomtnId;
    }

    public int getPostAutomtnId() {
        return postAutomtnId;
    }

    public void setPostAutomtnId(int postAutomtnId) {
        this.postAutomtnId = postAutomtnId;
    }

    public String getResourceURI() {
        return resourceURI;
    }

    public void setResourceURI(String resourceURI) {
        this.resourceURI = resourceURI;
    }

    public String getHost() {
        return host;
    }

    public void setHost(String host) {
        this.host = host;
    }

    public LinkedList<String> getResourceTypes() {
        return resourceTypes;
    }

    public void setResourceTypes(LinkedList<String> resourceTypes) {
        this.resourceTypes = resourceTypes;
    }

    public LinkedList<String> getResourceInterfaces() {
        return resourceInterfaces;
    }

    public void setResourceInterfaces(LinkedList<String> resourceInterfaces) {
        this.resourceInterfaces = resourceInterfaces;
    }

    public SimulatorConnectivityType getConnectivityType() {
        return connectivityType;
    }

    public void setConnectivityType(SimulatorConnectivityType connectivityType) {
        this.connectivityType = connectivityType;
    }

    public boolean isObservable() {
        return isObservable;
    }

    public void setObservable(boolean isObservable) {
        this.isObservable = isObservable;
    }

    public boolean isGetAutomtnInProgress() {
        return getAutomtnInProgress;
    }

    public void setGetAutomtnInProgress(boolean getAutomtnInProgress) {
        this.getAutomtnInProgress = getAutomtnInProgress;
    }

    public boolean isPutAutomtnInProgress() {
        return putAutomtnInProgress;
    }

    public void setPutAutomtnInProgress(boolean putAutomtnInProgress) {
        this.putAutomtnInProgress = putAutomtnInProgress;
    }

    public boolean isPostAutomtnInProgress() {
        return postAutomtnInProgress;
    }

    public void setPostAutomtnInProgress(boolean postAutomtnInProgress) {
        this.postAutomtnInProgress = postAutomtnInProgress;
    }

    public boolean isConfigUploaded() {
        return configUploaded;
    }

    public void setConfigUploaded(boolean configUploaded) {
        this.configUploaded = configUploaded;
    }

    public SimulatorRemoteResource getResource() {
        return resourceN;
    }

    public void setResource(SimulatorRemoteResource resource) {
        this.resourceN = resource;
    }

    public boolean isObserved() {
        return observed;
    }

    public void setObserved(boolean observed) {
        this.observed = observed;
    }

    public List<PutPostAttributeModel> getPutPostModel() {
        Map<String, RemoteResourceAttribute> attMap = getResourceAttributesMap();
        if (null == attMap || attMap.size() < 1) {
            return null;
        }
        List<PutPostAttributeModel> putPostModelList = new ArrayList<PutPostAttributeModel>();
        String attName;
        RemoteResourceAttribute attribute;
        PutPostAttributeModel putPostModel;
        Iterator<String> attItr = attMap.keySet().iterator();
        while (attItr.hasNext()) {
            attName = attItr.next();
            attribute = attMap.get(attName);
            putPostModel = PutPostAttributeModel.getModel(attribute);
            if (null != putPostModel) {
                putPostModelList.add(putPostModel);
            }
        }
        return putPostModelList;
    }

    public String getAttributeValue(String attName) {
        RemoteResourceAttribute attribute = resourceAttributesMap.get(attName);
        if (null == attribute) {
            return null;
        }
        return String.valueOf(attribute.getAttributeValue());
    }

    public String getuId() {
        return uId;
    }

    public void setuId(String uId) {
        this.uId = uId;
    }

    public int getAutomationtype(int autoId) {
        if (getAutomtnId == autoId) {
            return Constants.GET_AUTOMATION_INDEX;
        } else if (putAutomtnId == autoId) {
            return Constants.PUT_AUTOMATION_INDEX;
        } else {// if(postAutomtnId == autoId) {
            return Constants.POST_AUTOMATION_INDEX;
        }
    }

    public void updateAutomationStatus(int autoId, boolean status) {
        if (getAutomtnId == autoId) {
            getAutomtnInProgress = status;
        } else if (putAutomtnId == autoId) {
            putAutomtnInProgress = status;
        } else {// if(postAutomtnId == autoId) {
            postAutomtnInProgress = status;
        }
    }

    public boolean isFavorite() {
        return isFavorite;
    }

    public void setFavorite(boolean isFavorite) {
        this.isFavorite = isFavorite;
    }
}
