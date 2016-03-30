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

import java.util.List;

import org.oic.simulator.ResourceAttribute;
import org.oic.simulator.ResourceAttribute.Range;
import org.oic.simulator.ResourceAttribute.Type;
import org.oic.simulator.serviceprovider.AutomationType;

/**
 * This class represents an attribute in the simulated resource.
 */
public class LocalResourceAttribute {

    // Native object reference
    private ResourceAttribute resourceAttribute;

    private Object            attributeValue;
    private List<String>      attValues;

    private int               automationId;

    private boolean           automationInProgress;

    private int               automationUpdateInterval;

    private AutomationType    automationType;

    public ResourceAttribute getResourceAttribute() {
        return resourceAttribute;
    }

    public void setResourceAttribute(ResourceAttribute resourceAttribute) {
        this.resourceAttribute = resourceAttribute;
    }

    public String getAttributeName() {
        return resourceAttribute.getName();
    }

    public Object getAttributeValue() {
        return attributeValue;
    }

    public void setAttributeValue(Object attributeValue) {
        this.attributeValue = attributeValue;
    }

    public Object[] getAllowedValues() {
        return resourceAttribute.getAllowedValues();
    }

    public Object getMinValue() {
        return resourceAttribute.getRange().getMin();
    }

    public Object getMaxValue() {
        return resourceAttribute.getRange().getMax();
    }

    public boolean isAutomationInProgress() {
        return automationInProgress;
    }

    public void setAutomationInProgress(boolean automationInProgress) {
        this.automationInProgress = automationInProgress;
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

    public Type getAttValType() {
        return resourceAttribute.getType();
    }

    public Type getAttValBaseType() {
        return resourceAttribute.getBaseType();
    }

    public List<String> getAttValues() {
        return attValues;
    }

    public void setAttValues(List<String> attValues) {
        this.attValues = attValues;
    }

    public void printAttributeDetails() {
        System.out.println("Attribute Name:" + resourceAttribute.getName());
        System.out.println("Attribute Value:" + resourceAttribute.getValue());
        System.out.println("Attribute Base Type:"
                + resourceAttribute.getBaseType());
        System.out.println("Attribute Type:" + resourceAttribute.getType());
        System.out.print("Allowed Values:");
        Object[] values = getAllowedValues();
        for (Object obj : values) {
            System.out.print(obj);
        }
        Range range = resourceAttribute.getRange();
        if (null != range) {
            System.out.println("Range:" + range.getMin() + " to "
                    + range.getMax());
        }
    }
}