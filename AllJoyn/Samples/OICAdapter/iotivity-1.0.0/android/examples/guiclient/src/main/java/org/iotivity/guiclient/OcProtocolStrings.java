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

/**
 * OcProtocolStrings contains the IoTivity-specific constant values.  To add another supported
 * Resource or Interface type to this app, begin by adding the new strings here, and then
 * find the places throughout the app where Resource-specific case switches occur, and add
 * the newly-supported type there.
 */
public interface OcProtocolStrings {
    // OIC core protocol strings
    public static final String COAP_CORE = "coap://224.0.1.187/oic/res";
    public static final String RESOURCE_TYPE_QUERY = "?rt=";
    public static final String INTERFACE_QUERY = "?if=";
    // find resource queries
    public static final String CORE_LIGHT = "core.light";
    public static final String CORE_EDISON_RESOURCES = "core.edison.resources";
    // resource URIs
    public static final String LIGHT_RESOURCE_URI = "/a/light";
    public static final String LIGHT_RESOURCE_URI2 = "/light0";
    public static final String LIGHT_RESOURCE_URI3 = "/a/light1";
    public static final String ROOM_TEMPERATURE_RESOURCE_URI = "/temperature";
    public static final String AMBIENT_LIGHT_RESOURCE_URI = "/ambientlight";
    public static final String PLATFORM_LED_RESOURCE_URI = "/led";
    // attribute keys for set() calls
    public static final String LIGHT_SWITCH_RESOURCE_KEY = "state";
    public static final String LIGHT_DIMMER_RESOURCE_KEY = "power";
    public static final String ROOM_TEMPERATURE_RESOURCE_KEY = "temperature";
    public static final String AMBIENT_LIGHT_RESOURCE_KEY = "ambientlight";
    public static final String PLATFORM_LED_RESOURCE_KEY = "switch";
}
