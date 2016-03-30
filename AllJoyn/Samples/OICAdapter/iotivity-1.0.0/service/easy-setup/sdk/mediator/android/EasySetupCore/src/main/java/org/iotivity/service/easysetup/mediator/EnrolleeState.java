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
 * It defines various states of the Enrollee device during easy setup process
 */

public enum EnrolleeState {

    /**
     * Default state of the device
     */
    DEVICE_INIT_STATE,

    /**
     * Device will move to this state after successful on-boarding of the device
     */
    DEVICE_ON_BOARDED_STATE,

    /**
     * Device will move to this state once the on boarding begins
     */
    DEVICE_ON_BOARDING_STATE,

    /**
     * Device will move to this state once the on boarding is done
     */
    DEVICE_PROVISIONING_STATE,

    /**
     * Easy setup process is successful.
     */
    DEVICE_PROVISIONED_STATE,

    /**
     * This state is arbitrary one, any time device can come into this state
     * Device will move to this state if the ownership transfer initiated  by the Application
     */
    DEVICE_OWNERSHIP_TRANSFERRING_STATE,

    /**
     * This state is arbitrary one, any time device can come into this state
     * Device will move to this state if the ownership transfer is completed
     */
    DEVICE_OWNERSHIP_TRANSFERRED_STATE,


    /**
     * This state is arbitrary one, any time device can come into this state
     * Device will move to this state once the Application factory reset the device
     */
    DEVICE_FACTORY_RESET_STATE,

}
