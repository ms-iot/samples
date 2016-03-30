/*
 * //******************************************************************
 * //
 * // Copyright 2015 Samsung Electronics All Rights Reserved.
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

import java.util.List;
import java.util.Arrays;
import java.util.Map;

/**
 * OcProvisionig represents functions corresponding to the provisioing of
 * resources.
 */
public class OcProvisioning {

    /**
     * Method to Intialize Provisioning Manager.This will load the provisioning
     * Manager's persistent database.
     *
     * @param dbPath     dbPath file path of the sqlite3 db.
     * @throws OcException
     */
    public static native void provisionInit(String dbPath) throws OcException;

    /**
     * Method to Discover un-owned devices in its subnet.Un-owned devices need
     * to be owned by calling ownershipTransferCBdata.
     *
     * @param timeout     Timeout in sec.Time to listen for responses before
     *                    returining the Array.
     * @return            Array of OcSecureResource class objects.
     * @throws OcException
     */
    public  static List<OcSecureResource> discoverUnownedDevices(int timeout) throws OcException {
        return Arrays.asList(OcProvisioning.discoverUnownedDevices1(timeout));
    }
    private static native OcSecureResource[] discoverUnownedDevices1(int timeout) throws OcException;

    /**
     * Method to Discover owned devices in its subnet.
     *
     * @param timeout     Timeout in sec.Time to listen for responses before
     *                    returining the Array.
     * @return            Array of OcSecureResource class objects.
     * @throws OcException
     */
    public static List<OcSecureResource> discoverOwnedDevices(int timeout) throws OcException {
        return Arrays.asList(OcProvisioning.discoverOwnedDevices1(timeout));
    }
    private static native OcSecureResource[] discoverOwnedDevices1(int timeout) throws OcException;

    /**
     *  API for registering Ownership transfer methods for a particular
     *  transfer Type
     *
     * @param type     OxmType ownership transfer type.
     * @throws OcException
     */
    public static void SetownershipTransferCBdata(OxmType type,
            PinCallbackListener pinCallbackListener) throws OcException
    {
        OcProvisioning.ownershipTransferCBdata(type.getValue(), pinCallbackListener);
    }

    private  static native void ownershipTransferCBdata(int oxmType,  PinCallbackListener pinCallbackListener);

    public static interface PinCallbackListener {
        public String pinCallbackListener();
    }

    /**
     * Server API to set Callback for Displaying stack generated PIN.
     *
     * @param DisplayPinListener Pin callback Listener to be registered.
     * @throws OcException
     */
    public static native void setDisplayPinListener(DisplayPinListener displayPinListener)
        throws OcException;

    public static interface DisplayPinListener {
        public void displayPinListener(String Pin);
    }

    /**
     * Method to get Array of owned and un-owned devices in the current subnet.
     *
     * @param timeout    timeout in sec for the API to return.
     * @retrun           Array of OcSecureResource class objects.
     *                   be provisioned.
     * @throws OcException
     */
    public static List<OcSecureResource> getDeviceStatusList(int timeout) throws OcException {
        return Arrays.asList(OcProvisioning.getDeviceStatusList1(timeout));
    }
    private static native OcSecureResource[] getDeviceStatusList1(int timeout) throws OcException;
}
