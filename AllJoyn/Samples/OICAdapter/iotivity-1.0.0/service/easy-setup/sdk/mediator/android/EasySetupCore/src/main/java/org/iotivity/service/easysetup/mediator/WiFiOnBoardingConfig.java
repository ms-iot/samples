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

import org.iotivity.service.easysetup.mediator.OnBoardingConfig;

import android.net.wifi.WifiConfiguration;

/**
 * This class contains on boarding configuration information for Soft AP on boarding connectivity.
 * It implements OnBoardingConfig interface & provide implementation for WiFi Soft AP connectivity.
 */

public class WiFiOnBoardingConfig implements OnBoardingConfig {

    private final WifiConfiguration config = new WifiConfiguration();
    private final ConnType mConnType = OnBoardingConfig.ConnType.WiFi;

    @Override
    public Object getConfig() {
        return config;
    }

    public void setSSId(String ssid) {
        config.SSID = ssid;
    }

    public void setSharedKey(String sharedKey) {
        config.preSharedKey = sharedKey;
    }

    public void setAuthAlgo(int aurthAlgo) {
        config.allowedAuthAlgorithms.set(aurthAlgo);
    }

    public void setKms(int kms) {
        config.allowedKeyManagement.set(kms);
    }

    @Override
    public ConnType getConnType() {
        return mConnType;
    }
}
