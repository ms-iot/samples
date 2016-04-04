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

import android.content.Context;
import android.media.Image;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseExpandableListAdapter;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;

import java.util.List;

import static org.iotivity.guiclient.OcAttributeInfo.OC_ATTRIBUTE_TYPE;
import static org.iotivity.guiclient.OcProtocolStrings.AMBIENT_LIGHT_RESOURCE_URI;
import static org.iotivity.guiclient.OcProtocolStrings.LIGHT_RESOURCE_URI;
import static org.iotivity.guiclient.OcProtocolStrings.PLATFORM_LED_RESOURCE_URI;
import static org.iotivity.guiclient.OcProtocolStrings.ROOM_TEMPERATURE_RESOURCE_URI;

/**
 * ExpandableResourceListAdapter knows how to render an ExpandableListView, using a
 * List of OcResourceInfo objects as the parents of the ExpandableListView,
 * and OcAttributeInfo objects as the children.
 *
 * @see org.iotivity.guiclient.OcAttributeInfo
 */
public class ExpandableResourceListAdapter extends BaseExpandableListAdapter {
    /**
     * Hardcoded TAG... if project never uses proguard then
     * MyOcClient.class.getName() is the better way.
     */
    private static final String TAG = "ExpandableResourceListAdapter";

    private static final boolean LOCAL_LOGV = true; // set to false to compile out verbose logging

    private List<OcResourceInfo> resourceList;
    private Context ctx;

    public ExpandableResourceListAdapter(List<OcResourceInfo> resourceList, Context ctx) {
        this.resourceList = resourceList;
        this.ctx = ctx;
    }

    @Override
    public Object getChild(int groupPosition, int childPosition) {
        return resourceList.get(groupPosition).getAttributes().get(childPosition);
    }

    @Override
    public int getChildrenCount(int groupPosition) {
        return resourceList.get(groupPosition).getAttributes().size();
    }

    @Override
    public long getChildId(int groupPosition, int childPosition) {
        return resourceList.get(groupPosition).getAttributes().get(childPosition).hashCode();
    }

    @Override
    public int getChildType(int groupPosition, int childPosition) {
        return this.resourceList.get(groupPosition).getAttributes().get(childPosition)
                .getType().ordinal();
    }

    @Override
    public int getChildTypeCount() {
        return OC_ATTRIBUTE_TYPE.values().length;
    }

    @Override
    public View getChildView(final int groupPosition, int childPosition,
                             boolean isLastChild, View convertView, ViewGroup parent) {
        View v = convertView;
        if (v == null) {
            LayoutInflater inflater = (LayoutInflater)ctx.getSystemService
                    (Context.LAYOUT_INFLATER_SERVICE);
            switch(OC_ATTRIBUTE_TYPE.fromInt(getChildType(groupPosition, childPosition))) {
                case AMBIENT_LIGHT_SENSOR_READING:
                case ROOM_TEMPERATURE_SENSOR_READING:
                    v = inflater.inflate(R.layout.attribute_layout_progress_bar, parent, false);
                    break;
                case LIGHT_DIMMER:
                    v = inflater.inflate(R.layout.attribute_layout_slider, parent, false);
                    break;
                case LIGHT_SWITCH:
                case PLATFORM_LED_SWITCH:
                    v = inflater.inflate(R.layout.attribute_layout_on_off_switch, parent, false);
                    break;
            }
        }

        OcAttributeInfo attribute =
                resourceList.get(groupPosition).getAttributes().get(childPosition);

        // All attribute icons and names are currently treated the same so we handle them outside
        // the type-specific inflater functions
        ImageView attributeIcon = (ImageView) v.findViewById(R.id.attribute_icon_id);
        attributeIcon.setVisibility(View.VISIBLE);
        TextView attributeName = (TextView) v.findViewById(R.id.attribute_name_id);
        attributeName.setText(getAttributeLabelFromType(attribute.getType()));
        attributeName.setVisibility(View.VISIBLE);

        // Now inflate the rest of the layout in a type-specific way
        switch(attribute.getType()){
            case AMBIENT_LIGHT_SENSOR_READING:
                this.renderAmbientLightSensorReading(v, groupPosition, attribute);
                break;
            case LIGHT_DIMMER:
                this.renderLightDimmer(v, groupPosition, attribute);
                break;
            case LIGHT_SWITCH:
                this.renderLightSwitch(v, groupPosition, attribute);
                break;
            case PLATFORM_LED_SWITCH:
                this.renderPlatformLedSwitch(v, groupPosition, attribute);
                break;
            case ROOM_TEMPERATURE_SENSOR_READING:
                this.renderRoomTemperatureSensorReading(v, groupPosition, attribute);
                break;
        }

        return v;
    }

    @Override
    public Object getGroup(int groupPosition) {
        return resourceList.get(groupPosition);
    }

    @Override
    public int getGroupCount() {
        return resourceList.size();
    }

    @Override
    public long getGroupId(int groupPosition) {
        return resourceList.get(groupPosition).hashCode();
    }

    @Override
    public View getGroupView(int groupPosition, boolean isExpanded,
                             View convertView, ViewGroup parent) {

        View v = convertView;

        if (v == null) {
            LayoutInflater inflater = (LayoutInflater)ctx.getSystemService
                    (Context.LAYOUT_INFLATER_SERVICE);
            v = inflater.inflate(R.layout.resource_list_item_layout, parent, false);
        }

        TextView resourceName = (TextView) v.findViewById(R.id.resource_name_id);
        TextView resourceDescription = (TextView) v.findViewById(R.id.resource_description_id);
        ImageView resourceIcon = (ImageView) v.findViewById(R.id.resource_icon_id);

        OcResourceInfo resource = resourceList.get(groupPosition);

        resourceName.setText(this.getResourceLabelFromType(resource.getType()));
        resourceDescription.setText(resource.getHost()+resource.getUri());
        switch (resource.getType()) {
            case AMBIENT_LIGHT_SENSOR:
                resourceIcon.setImageResource(R.drawable.iotivity_hex_icon);
                break;
            case LIGHT:
                resourceIcon.setImageResource(R.drawable.light_icon);
                break;
            case PLATFORM_LED:
                resourceIcon.setImageResource(R.drawable.led_icon);
                break;
            case ROOM_TEMPERATURE_SENSOR:
                resourceIcon.setImageResource(R.drawable.thermometer_icon);
                break;
            default:
                resourceIcon.setImageResource(R.drawable.iotivity_hex_icon);
                break;
        }

        return v;
    }

    @Override
    public boolean hasStableIds() {
        return true;
    }

    @Override
    public boolean isChildSelectable(int groupPosition, int childPosition) {
        return true;
    }

    /**
     * Type-specific layout render for Ambient Light Sensor reading attribute.
     */
    private void renderAmbientLightSensorReading(final View view,
                                                  final int groupPosition,
                                                  final OcAttributeInfo attribute) {
        // Render attributeValue
        TextView attributeValue = (TextView) view.findViewById(R.id.attribute_value_id);
        attributeValue.setText(String.valueOf(attribute.getValueInt()));
        attributeValue.setVisibility(View.VISIBLE);

        // Render progressBar
        ProgressBar progressBar = (ProgressBar) view.findViewById(R.id.attribute_progress_bar);
        progressBar.setMax(100); // display as percent from 0-100
        progressBar.setProgress(attribute.getValueAsPercentOfMax());
        progressBar.setVisibility(View.VISIBLE);
    }

    /**
     * Type-specific layout render for Light Dimmer attribute.
     */
    private void renderLightDimmer(final View view,
                                    final int groupPosition,
                                    final OcAttributeInfo attribute) {
        // Render attributeValue
        TextView attributeValue = (TextView) view.findViewById(R.id.attribute_value_id);
        attributeValue.setText(String.valueOf(attribute.getValueInt()));
        attributeValue.setVisibility(View.VISIBLE);

        // Render SeekBar
        SeekBar slider = (SeekBar) view.findViewById(R.id.attribute_slider);
        slider.setMax(attribute.getValueMax());
        slider.setProgress(attribute.getValueInt());
        slider.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            private int mSliderVal;
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (LOCAL_LOGV) Log.v(TAG, String.format("onProgressChanged(%s)", progress));
                this.mSliderVal = progress;
            }
            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}
            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                if (LOCAL_LOGV) Log.v(TAG, "onStopTrackingTouch()");
                if(ctx instanceof MainActivity) {
                    // call MainActivity
                    ((MainActivity) ctx).setLightDimmerLevel(resourceList.get(groupPosition),
                            this.mSliderVal);
                }
            }
        });
        slider.setVisibility(View.VISIBLE);
    }

    /**
     * Type-specific layout render for Light Switch attribute.
     */
    private void renderLightSwitch(final View view,
                                    final int groupPosition,
                                    final OcAttributeInfo attribute) {
        // Render attributeValue
        TextView attributeValue = (TextView) view.findViewById(R.id.attribute_value_id);
        if(false == attribute.getValueBool()) {
            attributeValue.setText("off");
        } else {
            attributeValue.setText("on");
        }
        attributeValue.setVisibility(View.VISIBLE);

        // Render Switch
        Switch toggleSwitch = (Switch) view.findViewById(R.id.attribute_switch);
        toggleSwitch.setText(this.ctx.getString(R.string.oc_light_switch_toggle_text));
        toggleSwitch.setChecked(attribute.getValueBool());
        toggleSwitch.setOnCheckedChangeListener(new Switch.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (LOCAL_LOGV) Log.v(TAG, String.format("onCheckedChanged(%s)", isChecked));
                if(ctx instanceof MainActivity) {
                    // call MainActivity
                    ((MainActivity) ctx).toggleLightSwitch(resourceList.get(groupPosition),
                            isChecked);
                }
            }
        });
        toggleSwitch.setVisibility(View.VISIBLE);
    }

    /**
     * Type-specific layout render for LED Switch attribute.
     */
    private void renderPlatformLedSwitch(final View view,
                                          final int groupPosition,
                                          final OcAttributeInfo attribute) {
        // Render attributeValue
        TextView attributeValue = (TextView) view.findViewById(R.id.attribute_value_id);
        if(1 == attribute.getValueInt()) {
            attributeValue.setText("on");
        } else {
            attributeValue.setText("off");
        }
        attributeValue.setVisibility(View.VISIBLE);

        // Render Switch
        Switch toggleSwitch = (Switch) view.findViewById(R.id.attribute_switch);
        toggleSwitch.setText(this.ctx.getString(R.string.oc_led_switch_toggle_text));
        toggleSwitch.setChecked(1 == attribute.getValueInt());
        toggleSwitch.setOnCheckedChangeListener( new Switch.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (LOCAL_LOGV) Log.v(TAG, String.format("onCheckedChanged(%s)", isChecked));
                if(ctx instanceof MainActivity) {
                    // call MainActivity
                    ((MainActivity) ctx).toggleLedSwitch(resourceList.get(groupPosition),
                            isChecked);
                }
            }
        });
        toggleSwitch.setVisibility(View.VISIBLE);
    }

    /**
     * Type-specific layout render for Room Temperature Sensor Reading attribute.
     */
    private void renderRoomTemperatureSensorReading(final View view,
                                                     final int groupPosition,
                                                     final OcAttributeInfo attribute) {
        // this happens to have the same behavior as ambient light sensor, so just re-use
        this.renderAmbientLightSensorReading(view, groupPosition, attribute);
    }

    private String getAttributeLabelFromType(OC_ATTRIBUTE_TYPE type) {
        switch (type) {
            case AMBIENT_LIGHT_SENSOR_READING:
                return ctx.getString(R.string.ui_attribute_label_ambient_light_sensor_reading);
            case LIGHT_DIMMER:
                return ctx.getString(R.string.ui_attribute_label_light_dimmer);
            case LIGHT_SWITCH:
                return ctx.getString(R.string.ui_attribute_label_light_switch);
            case PLATFORM_LED_SWITCH:
                return ctx.getString(R.string.ui_attribute_label_led_switch);
            case ROOM_TEMPERATURE_SENSOR_READING:
                return ctx.getString(R.string.ui_attribute_label_room_temperature_sensor_reading);
            default:
                Log.w(TAG, "getAttributeLabelFromType(): unrecognized attribute type.");
                return "Attribute:";
        }
    }

    private String getResourceLabelFromType(OcResourceInfo.OC_RESOURCE_TYPE type) {
        if (LOCAL_LOGV) Log.v(TAG, "getResourceLabelFromType()");

        switch(type) {
            case AMBIENT_LIGHT_SENSOR:
                return ctx.getString(R.string.ui_resource_label_ambient_light_sensor);
            case LIGHT:
                return ctx.getString(R.string.ui_resource_label_light);
            case PLATFORM_LED:
                return ctx.getString(R.string.ui_resource_label_platform_led);
            case ROOM_TEMPERATURE_SENSOR:
                return ctx.getString(R.string.ui_resource_label_room_temperature_sensor);
            default:
                Log.w(TAG, "getResourceLabelFromType(): unrecognized resource type.");
                return "Resource:";
        }
    }
}
