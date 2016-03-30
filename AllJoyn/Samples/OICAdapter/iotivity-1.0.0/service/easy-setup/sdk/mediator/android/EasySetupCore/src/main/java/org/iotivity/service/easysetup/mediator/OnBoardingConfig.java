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

import org.iotivity.base.OcConnectivityType;

/**
 * It contains configuration details about the connectivity type between the Enrollee device &
 * Mediator device in order to perform on-boarding.
 */

public interface OnBoardingConfig {

    /**
     * It provides constants for connectivity types used for on-boarding Enrollee device
     */
    public static enum ConnType {
        // Note : Enum Ids should matched with Native Ids
        WiFi(OcConnectivityType.CT_DEFAULT.getValue()),
        BLE(OcConnectivityType.CT_ADAPTER_GATT_BTLE.getValue());

        private int mConnType;

        ConnType(int connType) {
            mConnType = connType;
        }

        public int getValue() {
            return mConnType;
        }

    }

    /**
     * Gives configuration object specific to the on-boarding connectivity of the enrolling device.
     *
     * @return instance object of configuration according to connectivity type
     */
    public Object getConfig();

    /**
     * Gives connectivity type of on-boarding device
     *
     * @return on-boarding connectivity type
     */
    public ConnType getConnType();


}
