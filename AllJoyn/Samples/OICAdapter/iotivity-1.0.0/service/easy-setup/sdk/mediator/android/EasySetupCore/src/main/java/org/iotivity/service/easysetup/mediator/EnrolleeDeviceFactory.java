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

import android.content.Context;

/**
 * This a factory class provides the native implementation of the various Enrollee devices.
 * Application can make use of Enrollee factory if it does not want to create its own Enrollee devices.
 */
public class EnrolleeDeviceFactory {

    Context mContext;

    /**
     * This method create & returns instance of EnrolleeDeviceFactory
     *
     * @param context This is Android Application context
     */
    public static EnrolleeDeviceFactory newInstance(Context context) {
        return new EnrolleeDeviceFactory(context);
    }

    private EnrolleeDeviceFactory(Context context) {
        mContext = context;
    }

    /**
     * This method create & returns instance of Enrollee device of supported configuration
     *
     * @param onboardingConfig Contains details about the connectivity to be established between the Enrollee device & Mediator device in order to perform on-boarding
     * @param provConfig       Contains details about the network to which Enrollee device is going to connect.
     * @return Instance of the Enrollee device created natively.
     */

    public EnrolleeDevice newEnrolleeDevice(OnBoardingConfig onboardingConfig, ProvisioningConfig provConfig) {

        if (onboardingConfig.getConnType() != OnBoardingConfig.ConnType.WiFi) {
            throw new IllegalArgumentException("OnBoarding configuration is not supported");
        }

        EnrolleeDevice enrolleeDevice;
        enrolleeDevice = new EnrolleeDeviceWiFiOnboarding(mContext, onboardingConfig, provConfig);

        return enrolleeDevice;
    }

}
