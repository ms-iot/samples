/**
 * ***************************************************************
 * <p>
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 * <p>
 * <p>
 * <p>
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * <p>
 * http://www.apache.org/licenses/LICENSE-2.0
 * <p>
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * <p>
 * ****************************************************************
 */
package org.iotivity.service.easysetup.core;

import android.content.Context;

import org.iotivity.ca.CaInterface;

public class EasySetupManager {
    private native void InitEasySetup();

    private native void TerminateEasySetup();

    // TODO : "OcConnectivityType connectivityType" has to be passed as the
    // second parameter for PerformEasySetup
    // instead of integer
    private native void ProvisionEnrollee(String ipAddress, String netSSID,
                                          String netPWD, int connectivityType);

    private native void StopEnrolleeProvisioning(int connectivityType);

    public static native void initialize(Context context);

    private static EasySetupManager easySetupManagerInterfaceObj = null;
    private Context appContext = null;

    static {
        // Load Easy Setup JNI interface
        System.loadLibrary("gnustl_shared");
        System.loadLibrary("octbstack");
        System.loadLibrary("connectivity_abstraction");
        System.loadLibrary("easysetup-jni");
    }

    private EasySetupManager() {

    }

    /**
     * Function for Getting instance of EasySetupManager object.
     *
     * @return EasySetupManager instance.
     *
     */
    public static synchronized EasySetupManager getInstance() {
        if (null == easySetupManagerInterfaceObj) {
            easySetupManagerInterfaceObj = new EasySetupManager();
        }
        return easySetupManagerInterfaceObj;
    }

    public void setApplicationContext(Context context) {
        appContext = context;
    }

    public void initEasySetup() {
        CaInterface.initialize(appContext);
        InitEasySetup();
    }

    public void terminateEasySetup() {
        TerminateEasySetup();
    }

    public void provisionEnrollee(String ipAddress, String netSSID,
                                  String netPWD, int connectivityType) {

        ProvisionEnrollee(ipAddress, netSSID, netPWD, connectivityType);
    }

    public void stopEnrolleeProvisioning(int connectivityType) {
        StopEnrolleeProvisioning(connectivityType);
    }

}
