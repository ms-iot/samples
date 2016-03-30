/**
 * ***************************************************************
 * <p/>
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 * <p/>
 * <p/>
 * <p/>
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * <p/>
 * http://www.apache.org/licenses/LICENSE-2.0
 * <p/>
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * <p/>
 * ****************************************************************
 */

package org.iotivity.service.easysetup.mediator;

import android.net.wifi.WifiConfiguration;
import android.test.AndroidTestCase;

public class EnrolleeDeviceFactoryTest extends AndroidTestCase {

    EnrolleeDeviceFactory mFactory;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        mFactory = EnrolleeDeviceFactory.newInstance(getContext());
        assertTrue(mFactory != null);

    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
        mFactory = null;
    }


    public void testNewEnrolleeDevice_with_WiFiOnboarding() {

        /* Create On boarding configuration */
        WiFiOnBoardingConfig mWiFiOnBoardingConfig = new WiFiOnBoardingConfig();
        mWiFiOnBoardingConfig.setSSId("EasySetup123");
        mWiFiOnBoardingConfig.setSharedKey("EasySetup123");
        mWiFiOnBoardingConfig.setAuthAlgo(WifiConfiguration.AuthAlgorithm.OPEN);
        mWiFiOnBoardingConfig.setKms(WifiConfiguration.KeyMgmt.WPA_PSK);

        /* Create provisioning configuration */
        WiFiProvConfig mWiFiProvConfig = new WiFiProvConfig("hub2.4G", "11112222");

        /* Create enrolling device */
        EnrolleeDevice device = mFactory.newEnrolleeDevice(mWiFiOnBoardingConfig, mWiFiProvConfig);

        /* Check if the the device is created */
        assertTrue(device != null);

        /* Check if the the correct device is created as per the given configuration*/
        assertTrue((device instanceof EnrolleeDeviceWiFiOnboarding));

    }

}
