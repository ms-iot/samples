//******************************************************************
//
// Copyright 2014 Intel Corporation.
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

package org.iotivity.guiclient;

import android.util.Log;

import java.io.Serializable;

/**
 * Created by nathanhs on 12/28/15.
 */
public class OcAttributeInfo implements Serializable {
    /**
     * Hardcoded TAG... if project never uses proguard then
     * MyOcClient.class.getName() is the better way.
     */
    private static final String TAG = "OcAttributeInfo";

    private static final boolean LOCAL_LOGV = true; // set to false to compile out verbose logging

    private static final int AMBIENT_LIGHT_SENSOR_READING_MAX = 4096;
    private static final int LIGHT_DIMMER_MAX = 100;
    private static final int LIGHT_SWITCH_MAX = 1;
    private static final int PLATFORM_LED_SWITCH_MAX = 1;
    // TODO: once the temp getValueInt works, figure out how to scale this properly
    private static final int ROOM_TEMPERATURE_SENSOR_READING_MAX = 4096;

    /**
     * These are the resource types supported by OcResourceInfo.
     */
    public enum OC_ATTRIBUTE_TYPE {
        AMBIENT_LIGHT_SENSOR_READING,
        LIGHT_DIMMER,
        LIGHT_SWITCH,
        PLATFORM_LED_SWITCH,
        ROOM_TEMPERATURE_SENSOR_READING;

        private static final OC_ATTRIBUTE_TYPE[] values = OC_ATTRIBUTE_TYPE.values();

        public static OC_ATTRIBUTE_TYPE fromInt(int i) {
            return values[i];
        }
    }

    private boolean mCurrentlyObserved;
    private final int mId;
    private static int mIdInitializer = 0;
    private final boolean mObservable = true; //TODO BROKEN fix when implementing observe
    private final OcResourceInfo mParentResource;
    private final boolean mReadOnly;
    private final OC_ATTRIBUTE_TYPE mType;
    private boolean mValueBool; // used if attribute has a boolean value
    private int mValueInt; // used if attribute has an int value
    private String mValueString; // used if attribute has a String value
    private final int mValueMax;

    public OcAttributeInfo(OC_ATTRIBUTE_TYPE type, OcResourceInfo parent) {
        if (LOCAL_LOGV) Log.v(TAG, "OcAttributeInfo() constructor");

        this.mId = OcAttributeInfo.mIdInitializer++; // give a unique Id from other OcResourceInfos
        this.mParentResource = parent;
        this.mReadOnly = this.getReadOnlyBasedOnType(type);
        this.mType = type;
        this.mValueBool = false;
        this.mValueInt = 0;
        this.mValueString = "error";
        this.mValueMax = this.getMaxAttributeValueBasedOnType(type);
    }

    public int getId() {
        return mId;
    }

    public OC_ATTRIBUTE_TYPE getType() {
        return mType;
    }

    public boolean getValueBool() {
        return this.mValueBool;
    }

    public int getValueInt() {
        return this.mValueInt;
    }

    public int getValueMax() {
        return mValueMax;
    }

    public String getValueString() {
        return this.mValueString;
    }

    public int getValueAsPercentOfMax() {
        return (this.mValueInt*100)/this.mValueMax;
    }

    public boolean isCurrentlyObserved() {
        return this.mCurrentlyObserved;
    }

    public boolean isObservable() {
        return this.mObservable;
    }

    public boolean isReadOnly() {
        return mReadOnly;
    }

    public void setCurrentlyObserved(boolean currentlyObserved) {
        this.mCurrentlyObserved = currentlyObserved;
    }

    public void setValueBool(boolean value) {
        this.mValueBool = value;
    }

    public void setValueInt(int value) {
        this.mValueInt = value;
    }

    public void setValueString(String value) {
        this.mValueString = value;
    }

    private int getMaxAttributeValueBasedOnType(OC_ATTRIBUTE_TYPE type) {
        switch(type) {
            case AMBIENT_LIGHT_SENSOR_READING:
                return AMBIENT_LIGHT_SENSOR_READING_MAX;
            case LIGHT_DIMMER:
                return LIGHT_DIMMER_MAX;
            case LIGHT_SWITCH:
                return LIGHT_SWITCH_MAX;
            case PLATFORM_LED_SWITCH:
                return PLATFORM_LED_SWITCH_MAX;
            case ROOM_TEMPERATURE_SENSOR_READING:
                return ROOM_TEMPERATURE_SENSOR_READING_MAX;
            default:
                Log.w(TAG, "getMaxAttributeValueBasedOnType(): unrecognized attribute type.");
                return 0;
        }
    }

    private boolean getReadOnlyBasedOnType(OC_ATTRIBUTE_TYPE type) {
        switch(type) {
            case AMBIENT_LIGHT_SENSOR_READING:
            case ROOM_TEMPERATURE_SENSOR_READING:
                return true;
            case LIGHT_DIMMER:
            case LIGHT_SWITCH:
            case PLATFORM_LED_SWITCH:
                return false;
            default:
                Log.w(TAG, "getReadOnlyBasedOnType(): unrecognized attribute type.");
                return false;
        }
    }
}
