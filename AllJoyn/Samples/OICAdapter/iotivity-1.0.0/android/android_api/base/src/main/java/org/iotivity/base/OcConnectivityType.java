/*
 * //******************************************************************
 * //
 * // Copyright 2015 Intel Corporation.
 * //
 * //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * //
 * // Licensed under the Apache License, Version 2.0 (the "License");
 * // you may not use this file except in compliance with the License.
 * // You may obtain a copy of the License at
 * //
 * //      http://www.apache.org/licenses/LICENSE-2.0
 * //
 * // Unless required by applicable law or agreed to in writing, software
 * // distributed under the License is distributed on an "AS IS" BASIS,
 * // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * // See the License for the specific language governing permissions and
 * // limitations under the License.
 * //
 * //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

package org.iotivity.base;

import java.security.InvalidParameterException;
import java.util.EnumSet;

public enum OcConnectivityType {
    /** use when defaults are ok. */
    CT_DEFAULT              (0),

    /** IPv4 and IPv6, including 6LoWPAN.*/
    CT_ADAPTER_IP           (1 << 16),

    /** GATT over Bluetooth LE.*/
    CT_ADAPTER_GATT_BTLE    (1 << 17),

    /** RFCOMM over Bluetooth EDR.*/
    CT_ADAPTER_RFCOMM_BTEDR (1 << 18),

    /** Remote Access over XMPP.*/
    CT_ADAPTER_REMOTE_ACCESS(1 << 19),

    /** Insecure transport is the default (subject to change).*/

    /** secure the transport path.*/
    CT_FLAG_SECURE          (1 << 4),

    /** IPv4 & IPv6 autoselection is the default.*/

    /** IP adapter only.*/
    CT_IP_USE_V6            (1 << 5),

    /** IP adapter only.*/
    CT_IP_USE_V4            (1 << 6),

    /** Link-Local multicast is the default multicast scope for IPv6.
     * These are placed here to correspond to the IPv6 address bits.*/

    /** IPv6 Interface-Local scope(loopback).*/
    CT_SCOPE_INTERFACE      (0x1),

    /** IPv6 Link-Local scope (default).*/
    CT_SCOPE_LINK           (0x2),

    /** IPv6 Realm-Local scope.*/
    CT_SCOPE_REALM          (0x3),

    /** IPv6 Admin-Local scope.*/
    CT_SCOPE_ADMIN          (0x4),

    /** IPv6 Site-Local scope.*/
    CT_SCOPE_SITE           (0x5),

    /** IPv6 Organization-Local scope.*/
    CT_SCOPE_ORG            (0x8),

    /** IPv6 Global scope.*/
    CT_SCOPE_GLOBAL         (0xE),
    ;

    private int value;

    private OcConnectivityType(int value) {
        this.value = value;
    }

    public int getValue() {
        return this.value;
    }

    public static EnumSet<OcConnectivityType> convertToEnumSet(int value) {
        EnumSet<OcConnectivityType> typeSet = null;

        for (OcConnectivityType v : values()) {
            if (0 != (value & v.getValue())) {
                if (null == typeSet) {
                    typeSet = EnumSet.of(v);
                } else {
                    typeSet.add(v);
                }
            }
        }

        if (null == typeSet || typeSet.isEmpty()) {
            throw new InvalidParameterException("Unexpected OcConnectivityType value:" + value);
        }

        return typeSet;
    }
}
