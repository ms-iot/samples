/* *****************************************************************
 *
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * *****************************************************************/
package org.iotivity.base;

public class ProvisionResult {

    private String mDeviceId;
    private int mResult;

    public ProvisionResult(String deviceId, int result)
    {
        this.mDeviceId = deviceId;
        this.mResult = result;
    }

    public String getDevId()
    {
        return this.mDeviceId;
    }
    public int getResult()
    {
        return this.mResult;
    }
}
