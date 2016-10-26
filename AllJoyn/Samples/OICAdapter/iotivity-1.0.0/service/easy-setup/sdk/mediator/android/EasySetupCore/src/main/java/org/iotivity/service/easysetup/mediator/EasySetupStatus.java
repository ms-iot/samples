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
 * This interface facilitates Application to get progress & result of Easy setup
 * process Easy setup process.
 */
public interface EasySetupStatus {

    /**
     * This method is called back when easy setup process is finished. Easy
     * setup process status can read from the APIs given in Enrollee class.This
     * method is called on worker thread, UI operations to be executed on main /
     * Ui Thread.
     *
     * @param enrolledevice Device to be enrolled in network through Easy setup process
     */
    public void onFinished(EnrolleeDevice enrolledevice);

    /**
     * This method is called back to give intermediate progress on the easy
     * setup process This method is called on worker thread, UI operations to be
     * executed on main / Ui Thread.
     *
     * @param enrolleeDevice Gives state of the device changed during easy setup process
     */
    public void onProgress(EnrolleeDevice enrolleeDevice);

}
