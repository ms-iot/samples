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
import java.util.EnumSet;
import java.util.Map;

public class OcSecureResource {

    private OcSecureResource(long nativeHandle) {
        this.mNativeHandle = nativeHandle;
    }

    /**
     *  Method to Start Ownership Transfer of an un-owned device.
     *
     *  @param DoOwnershipTransferListener  Callback function, which will be called after
     *                                      completion of ownership Transfer.
     *  @throws OcException
     */
    public native void doOwnershipTransfer(DoOwnershipTransferListener doOwnershipTransferListener)
        throws OcException;

    /**
     *  Method removes device credential from all devices in subnet
     *
     *  @param timeout
     *  @param RemoveDeviceListener         Callback function, which will be called after
     *                                      completion of removing device.
     *  @throws OcException
     */
    public native void removeDevice(int timeout,  RemoveDeviceListener removeDeviceListener)
        throws OcException;

    /**
     *  Method removes the credential & relationship between the two devices.
     *
     *  @param jobject                      Second device
     *  @param UnlinkDevicesListener        Callback function, which will be called after
     *                                      completion of removing device.
     *  @throws OcException
     */
    public native void unlinkDevices(Object device2, UnlinkDevicesListener unlinkDevicesListener)
        throws OcException;

    /**
     *  Method removes the credential & relationship between the two devices.
     *
     *  @param EnumSet<CredType>            OR'ed Cred Types
     *  @param KeySize                          keySize
     *  @param Object                       Second device
     *  @param ProvisionCredentialsListener Callback function, which will be called after
     *                                      completion of removing device.
     *  @throws OcException
     */
    public void provisionCredentials(EnumSet<CredType> credTypeSet, KeySize keysize, Object device2,
            ProvisionCredentialsListener provisionCredentialsListener) throws OcException {
        int credTypeInt = 0;

        for (CredType credType : CredType.values()) {
            if (credTypeSet.contains(credType))
                credTypeInt |= credType.getValue();
        }
        this.provisionCredentials1(credTypeInt, keysize.getValue(),
                device2, provisionCredentialsListener);
    }
    private native void provisionCredentials1(int type, int keySize, Object device2,
            ProvisionCredentialsListener provisionCredentialsListener)
        throws OcException;

    /**
     *  Method send ACL information to resource.
     *
     *  @param jobject                      Acl
     *  @param ProvisionAclListener         Callback function, which will be called after
     *                                      completion of removing device.
     *  @throws OcException
     */
    public native void provisionACL(Object acl, ProvisionAclListener provisionACLListener)
        throws OcException;


    /**
     *  Method provisions credentials between two devices and ACLs for the devices who
     *  act as a server.
     *
     *  @param EnumSet<CredType>            OR'ed Cred Types
     *  @param KeySize                      keySize
     *  @param Object                       First acl
     *  @param Object                       Second device
     *  @param Object                       Second acl
     *  @param ProvisionPairwiseDevicesListener Callback function, which will be called after
     *                                      completion of removing device.
     *  @throws OcException
     */
    public void provisionPairwiseDevices(EnumSet<CredType> credTypeSet, KeySize keysize, Object acl1,
            Object device2, Object acl2,
            ProvisionPairwiseDevicesListener provisionPairwiseDevicesListener) throws OcException {
        int credTypeInt = 0;

        for (CredType credType : CredType.values()) {
            if (credTypeSet.contains(credType))
                credTypeInt |= credType.getValue();
        }
        this.provisionPairwiseDevices1(credTypeInt, keysize.getValue(), acl1, device2,
                acl2, provisionPairwiseDevicesListener);
    }
    private native void provisionPairwiseDevices1(int type, int keySize, Object acl1,
            Object device2, Object acl2,
            ProvisionPairwiseDevicesListener provisionPairwiseDevicesListener) throws OcException;

    /**
     * doOwnershipTransferListener can be registered with doOwnershipTransfer
     * call.
     * Listener notified asynchronously.
     */
    public interface DoOwnershipTransferListener {
        public void doOwnershipTransferListener(List<ProvisionResult> provisionResultList,
                int hasError);
    }

    /**
     * removeDeviceListener can be registered with removeDeviceListener
     * call.
     * Listener notified asynchronously.
     */
    public interface RemoveDeviceListener {
        public void removeDeviceListener(List<ProvisionResult> provisionResultList,
                int hasError);
    }

    /**
     * unlinkDevicesListener can be registered with unlinkDevicesListener
     * call.
     * Listener notified asynchronously.
     */
    public interface UnlinkDevicesListener {
        public void unlinkDevicesListener(List<ProvisionResult> provisionResultList,
                int hasError);
    }

    /**
     * provisionCredentialsListener can be registered with provisionCredentialsListener
     * call.
     * Listener notified asynchronously.
     */
    public interface ProvisionCredentialsListener {
        public void provisionCredentialsListener(List<ProvisionResult> provisionResultList,
                int hasError);
    }

    /**
     * provisionAclListener can be registered with provisionAclListener
     * call.
     * Listener notified asynchronously.
     */
    public interface ProvisionAclListener {
        public void provisionAclListener(List<ProvisionResult> provisionResultList,
                int hasError);
    }

    /**
     * provisionPairwiseDevicesListener can be registered with provisionPairwiseDevicesListener
     * call.
     * Listener notified asynchronously.
     */
    public interface ProvisionPairwiseDevicesListener {
        public void provisionPairwiseDevicesListener(List<ProvisionResult> provisionResultList,
                int hasError);
    }

    /** Method to get List of device ID of devices linked with invoking device.
     *
     *  @return Sring List  List of device id's of linked devices.
     */
     public native List<String> getLinkedDevices();

     /**
      * Method to get IP address of sercure discovered device.
      * @return Stringified IP address.
      */
    public native String getIpAddr();

    /**
     * Method to get device id of a device.
     * @return  Device ID of the selected device.
     */
    public native String getDeviceID();

    /**
     * Method to get device status (ON/OFF) of a device.
     * @return      ON/OFF
     */

    public DeviceStatus getDeviceStatus() throws OcException {
        return DeviceStatus.convertDeviceStatus(this.deviceStatus());
    }
    public native int deviceStatus() throws OcException;

    /**
     * Method to get device  ownership (OWNED/UNOWNED) status.
     * @return      OWNED/UNOWNED
     */

    public OwnedStatus getOwnedStatus() throws OcException {
        return  OwnedStatus.convertOwnedStatus(this.ownedStatus());
    }
    public native int ownedStatus() throws OcException;

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        dispose();
    }

    private native void dispose();

    private long mNativeHandle;
}
