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
/**
 * It contains configuration details about the the target network where Enrollee device is
 * going to be enrolled.
 */
public interface ProvisioningConfig {

    /**
     * It provides constants for connectivity types of target network
     */
    public static enum ConnType {
        WiFi,
        BT
    }

    /**
     * Gives the instance of the configuration object created according to the connectivity
     * type of target network.
     *
     * @return instance object of configuration according to connectivity type of target network
     */
    Object getConfig();


    /**
     * Gives connectivity type of target network
     *
     * @return Connectivity type of target network
     */
    ConnType getConnType();

}
