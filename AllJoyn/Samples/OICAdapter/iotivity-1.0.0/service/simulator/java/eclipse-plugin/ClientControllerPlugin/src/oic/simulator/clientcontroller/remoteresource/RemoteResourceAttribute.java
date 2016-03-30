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
import java.util.List;
import java.util.Map;

import org.oic.simulator.ResourceAttribute;
import org.oic.simulator.ResourceAttribute.Type;

/**
 * This class represents an attribute in the remote resource.
 */
public class RemoteResourceAttribute {

    // Native object reference
    private ResourceAttribute resourceAttribute;

    private String            attributeName;
    private Object            attributeValue;
    private Type              attValType;
    private Type              attValBaseType;
    private List<Object>      allowedValues;

    private Object            minValue;
    private Object            maxValue;

    public ResourceAttribute getResourceAttribute() {
        return resourceAttribute;
    }

    public void setResourceAttribute(ResourceAttribute resourceAttribute) {
        this.resourceAttribute = resourceAttribute;
    }

    public String getAttributeName() {
        return attributeName;
    }

    public void setAttributeName(String attributeName) {
        this.attributeName = attributeName;
    }

    public Object getAttributeValue() {
        return attributeValue;
    }

    public void setAttributeValue(Object attributeValue) {
        this.attributeValue = attributeValue;
    }

    public List<Object> getAllowedValues() {
        return allowedValues;
    }

    public void setAllowedValues(List<Object> allowedValues) {
        this.allowedValues = allowedValues;
    }

    public void setAllowedValues(String[] allowedValues) {
        List<Object> allowedValueList = null;
        if (null != allowedValues && allowedValues.length > 0) {
            allowedValueList = new ArrayList<Object>();
            for (String value : allowedValues) {
                allowedValueList.add(value);
            }
        }
        this.allowedValues = allowedValueList;
    }

    public Object getMinValue() {
        return minValue;
    }

    public void setMinValue(Object minValue) {
        this.minValue = minValue;
    }

    public Object getMaxValue() {
        return maxValue;
    }

    public void setMaxValue(Object maxValue) {
        this.maxValue = maxValue;
    }

    public static RemoteResourceAttribute clone(
            RemoteResourceAttribute attribute) {
        RemoteResourceAttribute clone = null;
        if (null != attribute) {
            clone = new RemoteResourceAttribute();
            clone.setAttributeName(attribute.getAttributeName());
            clone.setAttributeValue(attribute.getAttributeValue());
            clone.setAllowedValues(attribute.getAllowedValues());
            clone.setAttValBaseType(attribute.getAttValBaseType());
            clone.setAttValType(attribute.getAttValType());
            clone.setMinValue(attribute.getMinValue());
            clone.setMaxValue(attribute.getMaxValue());
            clone.setResourceAttribute(null);
        }
        return clone;
    }

    // This method gives all known possible values of the attribute in string
    // format.
    // It takes allowed values or range of values whichever is available
    public List<String> getAllValues() {
        List<String> valueList = new ArrayList<String>();
        if (null != allowedValues) {
            Iterator<Object> values = allowedValues.iterator();
            Object value;
            while (values.hasNext()) {
                value = values.next();
                if (null != value) {
                    valueList.add(String.valueOf(value));
                }
            }
        } else if (null != minValue && null != maxValue) {
            if (attributeValue.getClass() == Integer.class) {
                int min = (Integer) minValue;
                int max = (Integer) maxValue;
                for (int value = min; value <= max; value++) {
                    valueList.add(String.valueOf(value));
                }
            } else if (attributeValue.getClass() == Double.class) {
                double min = (Double) minValue;
                double max = (Double) maxValue;
                for (double value = min; value <= max; value++) {
                    valueList.add(String.valueOf(value));
                }
            }
        }
        if (valueList.size() < 1 && null != attributeValue) {
            valueList.add(String.valueOf(attributeValue));
        }
        return valueList;
    }

    public static void printAttributes(
            Map<String, RemoteResourceAttribute> attributeMap) {
        Iterator<String> itr = attributeMap.keySet().iterator();
        String attName;
        RemoteResourceAttribute att;
        while (itr.hasNext()) {
            attName = itr.next();
            att = attributeMap.get(attName);
            System.out.println("AttributeName:" + attName);
            System.out.println("AttributeValue:"
                    + att.getAttributeValue().toString());
            System.out.println("Allowed Values:" + att.getAllValues());
        }
    }

    public Type getAttValType() {
        return attValType;
    }

    public void setAttValType(Type attValType) {
        this.attValType = attValType;
    }

    public Type getAttValBaseType() {
        return attValBaseType;
    }

    public void setAttValBaseType(Type attValBaseType) {
        this.attValBaseType = attValBaseType;
    }
}