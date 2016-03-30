//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
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
package org.iotivity.service.easysetup.mediator;

import android.net.wifi.WifiConfiguration;
import android.test.AndroidTestCase;
import android.util.Log;

import java.io.IOException;

public class EasySetupStatusTest extends AndroidTestCase {


    public void testStartSetupWithWiFiOnboarding() {

        EasySetupService mService;
        EnrolleeDevice mDevice;
        EnrolleeDeviceFactory mFactory;

        final Object lock = new Object();


        /* Create Easy Setup service */
        mService = EasySetupService.getInstance(getContext(), new EasySetupStatus() {
            EnrolleeState old_state = null;

            @Override
            public void onFinished(EnrolleeDevice enrolledevice) {

                //countDownLatch.countDown();
                Utility.toNotify(lock);

                if (enrolledevice.isSetupSuccessful()) {

                    if (enrolledevice.mOnBoardingConfig.getConnType() == WiFiOnBoardingConfig.ConnType.WiFi) {
                        IpOnBoardingConnection conn = (IpOnBoardingConnection) enrolledevice.getConnection();
                        String ip = conn.getIp();
                        if (ip == null || ip.isEmpty()) {
                            assertTrue(false);
                            return;
                        }
                        String mac = conn.getHardwareAddress();
                        if (mac == null || mac.isEmpty()) {
                            assertTrue(false);
                            return;
                        }
                        // Device configured successfully
                        assertTrue(true);
                    }

                } else {
                    assertTrue(false);
                }
            }

            @Override
            public void onProgress(EnrolleeDevice enrolleeDevice) {
                EnrolleeState state = enrolleeDevice.mState;
                // TODO
                switch (state) {
                    case DEVICE_INIT_STATE:
                        Log.d("enrollee state", "DEVICE_INIT_STATE");
                        assertTrue(false);
                        break;
                    case DEVICE_ON_BOARDING_STATE:
                        if (old_state == null)
                            assertTrue(true);
                        else assertTrue(false);
                        old_state = EnrolleeState.DEVICE_ON_BOARDING_STATE;
                        Log.d("enrollee state", "DEVICE_ON_BOARDING_STATE");
                        break;

                    case DEVICE_ON_BOARDED_STATE:
                        if (old_state == EnrolleeState.DEVICE_ON_BOARDING_STATE)
                            assertTrue(true);
                        else assertTrue(false);
                        old_state = EnrolleeState.DEVICE_ON_BOARDED_STATE;
                        Log.d("enrollee state", "DEVICE_ON_BOARDED_STATE");
                        break;

                    case DEVICE_PROVISIONING_STATE:
                        if (old_state == EnrolleeState.DEVICE_ON_BOARDED_STATE)
                            assertTrue(true);
                        else assertTrue(false);
                        old_state = EnrolleeState.DEVICE_PROVISIONING_STATE;
                        Log.d("enrollee state", "DEVICE_PROVISIONING_STATE");
                        break;

                    case DEVICE_PROVISIONED_STATE:
                        if (old_state == EnrolleeState.DEVICE_PROVISIONING_STATE)
                            assertTrue(true);
                        else assertTrue(false);
                        Log.d("enrollee state", "DEVICE_PROVISIONING_SUCCESS_STATE");
                        break;

                    default:
                        Log.d("enrollee state", "unknown state");
                        assertTrue(false);
                        break;
                }

            }
        });


        /* Create On boarding configuration */
        WiFiOnBoardingConfig mWiFiOnBoardingConfig = new WiFiOnBoardingConfig();
        mWiFiOnBoardingConfig.setSSId("EasySetup123");
        mWiFiOnBoardingConfig.setSharedKey("EasySetup123");
        mWiFiOnBoardingConfig.setAuthAlgo(WifiConfiguration.AuthAlgorithm.OPEN);
        mWiFiOnBoardingConfig.setKms(WifiConfiguration.KeyMgmt.WPA_PSK);

        /* Create provisioning configuration */
        WiFiProvConfig mWiFiProvConfig = new WiFiProvConfig("hub2.4G", "11112222");

        /* Create enrolling device factory instance */
        mFactory = EnrolleeDeviceFactory.newInstance(getContext());

        /* Create enrolling device */
        mDevice = mFactory.newEnrolleeDevice(mWiFiOnBoardingConfig, mWiFiProvConfig);

        try {
            mService.startSetup(mDevice);
        } catch (IOException e) {
        }

        try {

            Utility.toWait(lock);

            Log.i("EasySetupTest", "Lock is released");

            IpOnBoardingConnection conn = (IpOnBoardingConnection) mDevice.getConnection();

            Log.i("EasySetupTest", "Ip" + conn.getIp());
            Log.i("EasySetupTest", "MAC" + conn.getHardwareAddress());

            // Device configured successfully

        } catch (Exception e) {
            e.printStackTrace();
        }

    }

}