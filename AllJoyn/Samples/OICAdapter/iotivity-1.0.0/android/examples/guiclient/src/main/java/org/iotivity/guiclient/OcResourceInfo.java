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

import org.iotivity.base.ErrorCode;
import org.iotivity.base.OcException;
import org.iotivity.base.OcHeaderOption;
import org.iotivity.base.OcRepresentation;
import org.iotivity.base.OcResource;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import static org.iotivity.guiclient.OcAttributeInfo.OC_ATTRIBUTE_TYPE.AMBIENT_LIGHT_SENSOR_READING;
import static org.iotivity.guiclient.OcAttributeInfo.OC_ATTRIBUTE_TYPE.LIGHT_DIMMER;
import static org.iotivity.guiclient.OcAttributeInfo.OC_ATTRIBUTE_TYPE.LIGHT_SWITCH;
import static org.iotivity.guiclient.OcProtocolStrings.AMBIENT_LIGHT_RESOURCE_KEY;
import static org.iotivity.guiclient.OcProtocolStrings.AMBIENT_LIGHT_RESOURCE_URI;
import static org.iotivity.guiclient.OcProtocolStrings.LIGHT_DIMMER_RESOURCE_KEY;
import static org.iotivity.guiclient.OcProtocolStrings.LIGHT_RESOURCE_URI;
import static org.iotivity.guiclient.OcProtocolStrings.LIGHT_RESOURCE_URI2;
import static org.iotivity.guiclient.OcProtocolStrings.LIGHT_RESOURCE_URI3;
import static org.iotivity.guiclient.OcProtocolStrings.LIGHT_SWITCH_RESOURCE_KEY;
import static org.iotivity.guiclient.OcProtocolStrings.PLATFORM_LED_RESOURCE_KEY;
import static org.iotivity.guiclient.OcProtocolStrings.PLATFORM_LED_RESOURCE_URI;
import static org.iotivity.guiclient.OcProtocolStrings.ROOM_TEMPERATURE_RESOURCE_URI;

/**
 * OcResourceInfo is a wrapper object for the OcResource object. It implements the Resource-
 * specific callbacks, and abstracts the IoTivity implementation details from the application.
 *
 * In order to use OcResourceInfo, an application should implement the OcResourceInfoListener
 * interface, which is called when the OcResourceInfo changes in any meaningful way.
 */
public class OcResourceInfo
        implements OcResource.OnGetListener, OcResource.OnPutListener, Serializable {
    /**
     * Hardcoded TAG... if project never uses proguard then
     * MyOcClient.class.getName() is the better way.
     */
    private static final String TAG = "OcResourceInfo";

    private static final boolean LOCAL_LOGV = true; // set to false to compile out verbose logging

    /**
     * These are the resource types supported by OcResourceInfo.  They should have corresponding
     * URI strings in the OcProtocolStrings interface.
     */
    public enum OC_RESOURCE_TYPE {
        AMBIENT_LIGHT_SENSOR,
        LIGHT,
        PLATFORM_LED,
        ROOM_TEMPERATURE_SENSOR;

        private static final OC_RESOURCE_TYPE[] values = OC_RESOURCE_TYPE.values();

        public static OC_RESOURCE_TYPE fromInt(int i) {
            return values[i];
        }
    }

    private List<OcAttributeInfo> mAttributes;
    private final String mHost;
    private final int mId;
    private static int mIdInitializer = 0;
    private List<OcResourceInfoListener> mListeners;
    private final OcResource mResource;
    private final OC_RESOURCE_TYPE mType;
    private final String mUri;

    public interface OcResourceInfoListener {
        public void onResourceInfoChanged(OcResourceInfo resourceInfo);
    }


    public OcResourceInfo(OcResource resource, OcResourceInfoListener changeListener) {
        if (LOCAL_LOGV) Log.v(TAG, "OcResourceInfo() constructor");

        this.mAttributes = new ArrayList<>();
        this.mHost = resource.getHost();
        this.mId = OcResourceInfo.mIdInitializer++; // give a unique Id from other OcResourceInfos
        this.mListeners = new ArrayList<>();
        this.mListeners.add(changeListener);
        this.mResource = resource;
        this.mType = this.getResourceTypeFromUri(resource.getUri());
        this.mUri = resource.getUri();

    }

    public void registerListener(OcResourceInfoListener changeListener) {
        if(null == this.mListeners) {
            Log.e(TAG, "registerListener(): null mListeners List");
        } else {
            boolean alreadyRegistered = false;
            for(OcResourceInfoListener rl : this.mListeners) {
                if(changeListener == rl) {
                    alreadyRegistered = true;
                }
            }
            if(!alreadyRegistered) {
                this.mListeners.add(changeListener);
            }
        }
    }

    public OC_RESOURCE_TYPE getType() {
        return this.mType;
    }

    public String getHost() {
        return this.mHost;
    }

    public String getUri() {
        return this.mUri;
    }

    @Override
    public void onGetFailed(Throwable throwable) {
        if (throwable instanceof OcException) {
            OcException ocEx = (OcException) throwable;
            ErrorCode errCode = ocEx.getErrorCode();
            //do something based on errorCode
        }
        Log.e(TAG, throwable.toString());
    }

    @Override
    public void onPutFailed(Throwable throwable) {
        if (throwable instanceof OcException) {
            OcException ocEx = (OcException) throwable;
            ErrorCode errCode = ocEx.getErrorCode();
            //do something based on errorCode
        }
        Log.e(TAG, throwable.toString());
    }

    public List<OcAttributeInfo> getAttributes() {
        return this.mAttributes;
    }

    public void doOcGet() {
        if(null != this.mResource) {
            try {
                this.mResource.get(new HashMap<String, String>(), this);
            } catch (OcException e) {
                e.printStackTrace();
                Log.e(TAG, e.getMessage());
            }
        } else {
            Log.e(TAG, "doOcGet(): null mResource");
        }
    }

    public void doOcPut(OcAttributeInfo attribute) {
        if (LOCAL_LOGV) Log.v(TAG, "doOcPut()");

        if(attribute.isReadOnly()) {
            Log.w(TAG, String.format("doOcPut(): %s attribute is read only; skipping put!",
                    attribute.getType()));
        } else {
            if (null != this.mResource) {
                try {
                    OcRepresentation representation = new OcRepresentation();
                    switch (attribute.getType()) {
                        case AMBIENT_LIGHT_SENSOR_READING:
                            break;
                        case LIGHT_DIMMER:
                            representation.setValueInt(LIGHT_DIMMER_RESOURCE_KEY,
                                    attribute.getValueInt());
                            // This 'sw' logic is here because the current IoTivity Light forces
                            // the switch to "false" if the switch val is not specified.
                            boolean sw = true;
                            for(OcAttributeInfo ai : this.mAttributes) {
                                if(ai.getType() == LIGHT_SWITCH) {
                                    sw = ai.getValueBool();
                                }
                            }
                            representation.setValueBool(LIGHT_SWITCH_RESOURCE_KEY, sw);
                            break;
                        case LIGHT_SWITCH:
                            representation.setValueBool(LIGHT_SWITCH_RESOURCE_KEY,
                                    attribute.getValueBool());
                            break;
                        case PLATFORM_LED_SWITCH:
                            representation.setValueInt(PLATFORM_LED_RESOURCE_KEY,
                                    attribute.getValueInt());
                            break;
                        case ROOM_TEMPERATURE_SENSOR_READING:
                            break;
                        default:
                            break;
                    }
                    this.mResource.put(representation, new HashMap<String, String>(), this);
                } catch (OcException e) {
                    e.printStackTrace();
                    Log.e(TAG, e.getMessage());
                }
            } else {
                Log.e(TAG, "doOcGet(): null mResource");
            }
        }
    }

    private static Object onGetCompletedLock = new Object();
    @Override
//    public void onGetCompleted(HeaderOptions headerOptions, OcRepresentation ocRepresentation) {
    public void onGetCompleted(List<OcHeaderOption> headerOptionList,
                               OcRepresentation ocRepresentation) {
        synchronized (onGetCompletedLock) {
            if (LOCAL_LOGV) Log.v(TAG, "enter -> onGetCompleted()");
            if (LOCAL_LOGV) Log.v(TAG, String.format("\tthis = %s", this.toString()));
            if (LOCAL_LOGV) Log.v(TAG, String.format("\tthis.mType = %s", this.mType));

            this.mAttributes.clear();
            switch(this.mType) {
                case AMBIENT_LIGHT_SENSOR:
                    int ambientLightVal = ocRepresentation.getValueInt(AMBIENT_LIGHT_RESOURCE_KEY);
                    if (LOCAL_LOGV) Log.v(TAG,
                            String.format("%s int value of %s attribute = %d",
                                    mType, AMBIENT_LIGHT_RESOURCE_KEY, ambientLightVal));
                    OcAttributeInfo ambientAttribute = new OcAttributeInfo(
                            AMBIENT_LIGHT_SENSOR_READING, this);
                    ambientAttribute.setValueInt(ambientLightVal);
                    this.mAttributes.add(ambientAttribute);
                    break;
                case LIGHT:
                    // do switch first
                    boolean lightSwitchVal = ocRepresentation.getValueBool(LIGHT_SWITCH_RESOURCE_KEY);
                    OcAttributeInfo lightSwitchAttribute = new OcAttributeInfo(
                            LIGHT_SWITCH, this);
                    lightSwitchAttribute.setValueBool(lightSwitchVal);
                    this.mAttributes.add(lightSwitchAttribute);
                    // then dimmer
                    int dimmerVal = ocRepresentation.getValueInt(LIGHT_DIMMER_RESOURCE_KEY);
                    OcAttributeInfo dimmerAttribute = new OcAttributeInfo(
                            LIGHT_DIMMER, this);
                    dimmerAttribute.setValueInt(dimmerVal);
                    this.mAttributes.add(dimmerAttribute);
                    break;
                case PLATFORM_LED:
                    int ledVal = ocRepresentation.getValueInt(PLATFORM_LED_RESOURCE_KEY);
                    if (LOCAL_LOGV) Log.v(TAG,
                            String.format("%s int value of %s attribute = %d",
                                    mType, PLATFORM_LED_RESOURCE_KEY, ledVal));
                    OcAttributeInfo ledAttribute = new OcAttributeInfo(
                            OcAttributeInfo.OC_ATTRIBUTE_TYPE.PLATFORM_LED_SWITCH, this);
                    ledAttribute.setValueInt(ledVal);
                    this.mAttributes.add(ledAttribute);
                    break;
                case ROOM_TEMPERATURE_SENSOR:
                    int temperatureVal = 98;
                    Log.w(TAG, "getting 'temperature' value is causing crash;" +
                            " skipping and using 98.");
                    // TODO This call crashes in the native JNI code.  The example .cpp
                    //      app receives a double for the Room Temp key, but the Java API
                    //      doesn't support getValueDouble yet.
                    //      Debug crash when API fixed?
//                    temperatureVal = ocRepresentation.getValueInt(ROOM_TEMPERATURE_RESOURCE_KEY);
                    if (LOCAL_LOGV) Log.v(TAG,
                            String.format("%s int value of 'temperature' attribute = %d",
                                    mType, temperatureVal));
                    OcAttributeInfo temperatureAttribute = new OcAttributeInfo(
                            OcAttributeInfo.OC_ATTRIBUTE_TYPE.ROOM_TEMPERATURE_SENSOR_READING,
                            this);
                    temperatureAttribute.setValueInt(temperatureVal);
                    this.mAttributes.add(temperatureAttribute);
                    break;
            }
            this.notifyListeners();
            if (LOCAL_LOGV) Log.v(TAG, "exit <- onGetCompleted()");
        }
    }

    /**
     * Should be called whenever any Resource or Attribute values change on this object.
     */
    private void notifyListeners() {
        if (LOCAL_LOGV) Log.v(TAG, "notifyListeners()");

        for(OcResourceInfoListener l : this.mListeners) {
            l.onResourceInfoChanged(this);
        }
    }

    private Object onPutCompletedLock = new Object();
    @Override
    public void onPutCompleted(List<OcHeaderOption> headerOptionList,
                               OcRepresentation ocRepresentation) {
        synchronized (onPutCompletedLock) {
            if (LOCAL_LOGV) Log.v(TAG, "enter -> onPutCompleted()");
            if (LOCAL_LOGV) Log.v(TAG, String.format("\tthis = %s", this.toString()));
            if (LOCAL_LOGV) Log.v(TAG, String.format("\tthis.mType = %s", this.mType));

            switch(this.mType) {
                case AMBIENT_LIGHT_SENSOR:
                    Log.w(TAG, String.format("onPutCompleted(): %s is a readonly attribute type.",
                            this.mType));
                    break;
                case LIGHT:
                    // do switch first
                    boolean lightSwitchVal = ocRepresentation.getValueBool(LIGHT_SWITCH_RESOURCE_KEY);
                    for(OcAttributeInfo ai : this.mAttributes) {
                        if (ai.getType() == OcAttributeInfo.OC_ATTRIBUTE_TYPE.LIGHT_SWITCH) {
                            ai.setValueBool(lightSwitchVal);
                        }
                    }
                    // then dimmer
                    int dimmerVal = ocRepresentation.getValueInt(LIGHT_DIMMER_RESOURCE_KEY);
                    for(OcAttributeInfo ai : this.mAttributes) {
                        if (ai.getType() == OcAttributeInfo.OC_ATTRIBUTE_TYPE.LIGHT_DIMMER) {
                            ai.setValueInt(dimmerVal);
                        }
                    }
                    break;
                case PLATFORM_LED:
                    int value = ocRepresentation.getValueInt(PLATFORM_LED_RESOURCE_KEY);
                    for(OcAttributeInfo ai : this.mAttributes) {
                        if (ai.getType() == OcAttributeInfo.OC_ATTRIBUTE_TYPE.PLATFORM_LED_SWITCH) {
                            ai.setValueInt(value);
                        }
                    }
                    break;
                case ROOM_TEMPERATURE_SENSOR:
                    Log.w(TAG, String.format("onPutCompleted(): %s is a readonly attribute type.",
                            this.mType));
                    break;
            }
            this.notifyListeners();
            if (LOCAL_LOGV) Log.v(TAG, "exit <- onPutCompleted()");
        }
    }

    private OC_RESOURCE_TYPE getResourceTypeFromUri(String uri) {
        if (LOCAL_LOGV) Log.v(TAG, "getResourceTypeFromUri()");

        switch(uri) {
            case LIGHT_RESOURCE_URI:
            case LIGHT_RESOURCE_URI2:
            case LIGHT_RESOURCE_URI3:
                return OC_RESOURCE_TYPE.LIGHT;
            case AMBIENT_LIGHT_RESOURCE_URI:
                return OC_RESOURCE_TYPE.AMBIENT_LIGHT_SENSOR;
            case PLATFORM_LED_RESOURCE_URI:
                return OC_RESOURCE_TYPE.PLATFORM_LED;
            case ROOM_TEMPERATURE_RESOURCE_URI:
                return OC_RESOURCE_TYPE.ROOM_TEMPERATURE_SENSOR;
            default:
                Log.w(TAG, "getResourceTypeFromUri(): unsupported resource '" + uri + "'" );
        }
        return null;
    }
}
