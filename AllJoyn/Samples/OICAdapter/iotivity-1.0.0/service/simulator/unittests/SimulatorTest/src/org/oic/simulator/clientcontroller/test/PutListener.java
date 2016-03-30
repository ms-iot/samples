/*
 * Copyright 2015 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.oic.simulator.clientcontroller.test;

import java.util.concurrent.CountDownLatch;

import org.oic.simulator.SimulatorResourceModel;
import org.oic.simulator.clientcontroller.IPutListener;

/**
 * This class implements methods for receiving notification when
 * response is received for PUT request.
 */
public class PutListener implements IPutListener
{
    private CountDownLatch lockObject;
    private ListenerObject putListenerObject;

    public PutListener(CountDownLatch lockObject, ListenerObject putListenerObject)
    {
        this.lockObject = lockObject;
        this.putListenerObject = putListenerObject;
    }

    @Override
    public void onPutCompleted(String uId, SimulatorResourceModel representation)
    {
        if (null != putListenerObject) {
            putListenerObject.setuId(uId);
            putListenerObject.setRepresentation(representation);
        }

        lockObject.countDown();
    }

    @Override
    public void onPutFailed(Throwable ex)
    {
        if (null != putListenerObject)
            putListenerObject.setEx(ex);

        lockObject.countDown();
    }
}
