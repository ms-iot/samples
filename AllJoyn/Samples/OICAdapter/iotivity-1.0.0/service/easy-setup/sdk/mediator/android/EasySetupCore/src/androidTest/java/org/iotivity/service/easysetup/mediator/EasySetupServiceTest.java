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

public class EasySetupServiceTest extends AndroidTestCase {


    public void testStartSetupWithWiFiOnboarding() {

        EasySetupService mService;
        EnrolleeDevice mDevice;
        EnrolleeDeviceFactory mFactory;

        final Object lock = new Object();


        /* Create Easy Setup service */
        mService = EasySetupService.getInstance(getContext(), new EasySetupStatus() {
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
                // Handled in EasySetupStatusTest
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

        /* Check if the factory created successfully */
        assertTrue(mFactory != null);

        /* Create enrolling device */
        mDevice = mFactory.newEnrolleeDevice(mWiFiOnBoardingConfig, mWiFiProvConfig);

        /* Check if the the device is created successfully*/
        assertTrue(mDevice != null);

        /* Check if the the correct device is created as per the given configuration*/
        assertTrue((mDevice instanceof EnrolleeDeviceWiFiOnboarding));


        try {
            mService.startSetup(mDevice);
            // If no exception is thrown means setup started successfully.
            assertTrue(true);

        } catch (IOException e) {
            assertTrue(false);
        }

        try {

            Utility.toWait(lock);

            Log.i("EasySetupTest", "Lock is released");

            if (!mDevice.isSetupSuccessful()) {
                assertTrue(false);
                return;
            }

            IpOnBoardingConnection conn = (IpOnBoardingConnection) mDevice.getConnection();
            if (conn == null) {
                assertTrue(false);
                return;
            }

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

            Log.i("EasySetupTest", "Ip" + conn.getIp());
            Log.i("EasySetupTest", "MAC" + conn.getHardwareAddress());

            // Device configured successfully
            assertTrue(true);

        } catch (Exception e) {
            e.printStackTrace();
            assertTrue(false);
        }

    }

}