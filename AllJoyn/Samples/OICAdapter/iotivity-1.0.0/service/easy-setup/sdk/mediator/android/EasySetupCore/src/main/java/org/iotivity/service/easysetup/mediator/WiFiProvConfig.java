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

package org.iotivity.service.easysetup.mediator;

import org.iotivity.service.easysetup.mediator.ProvisioningConfig;

/**
 * This class contains on provisioning configuration information for for target network.
 * It implements ProvisioningConfig interface and provide configuration object specific to WiFi target network
 */
public class WiFiProvConfig implements ProvisioningConfig {

    private final ConnType mConnType = ProvisioningConfig.ConnType.WiFi;

    private final String mSsId;
    private final String mPassword;

    public WiFiProvConfig(String ssid, String pass) {
        mSsId = ssid;
        mPassword = pass;
    }

    @Override
    public Object getConfig() {
        return this;
    }

    @Override
    public ConnType getConnType() {
        return mConnType;
    }

    /**
     * This method returns the SSID of the Target WIFI network
     * @return SSID of Target Network
     */
    public String getSsId() {
        return mSsId;
    }

    /**
     * This method returns the password of the Target WIFI network
     * @return password of Target Network
     */
    public String getPassword() {
        return mPassword;
    }

}
