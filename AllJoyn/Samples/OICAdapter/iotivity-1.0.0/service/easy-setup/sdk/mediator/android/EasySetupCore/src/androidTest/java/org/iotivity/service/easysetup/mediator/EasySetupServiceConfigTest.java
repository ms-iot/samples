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

import android.test.AndroidTestCase;


public class EasySetupServiceConfigTest extends AndroidTestCase {


    public void testConstruction() {

        EasySetupService service = EasySetupService.getInstance(getContext(), new EasySetupStatus() {
            @Override
            public void onFinished(EnrolleeDevice enrolledevice) {

            }

            @Override
            public void onProgress(EnrolleeDevice enrolleeDevice) {

            }
        });

        assertTrue(service != null);


    }

    public void testFinish() {
        EasySetupService service = EasySetupService.getInstance(getContext(), new EasySetupStatus() {
            @Override
            public void onFinished(EnrolleeDevice enrolledevice) {

            }

            @Override
            public void onProgress(EnrolleeDevice enrolleeDevice) {

            }
        });
        service.finish();

        // No runtime exception is thrown means test is successful
        assertTrue(true);

    }

    public void testEnrolleeDeviceFacotryConstruction() {
        EnrolleeDeviceFactory factory = EnrolleeDeviceFactory.newInstance(getContext());
        assertTrue(factory != null);
    }

}
